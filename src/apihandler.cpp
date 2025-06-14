#include <apihandler.h>
#include "../include/log.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <format>
#include <zlib.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define CLOSESOCKET closesocket
    #define SOCK_ERR INVALID_SOCKET
    #define SOCK_INIT()  WSADATA wsaData; WSAStartup(MAKEWORD(2,2), &wsaData)
    #define SOCK_CLEAN() WSACleanup()
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define CLOSESOCKET close
    #define SOCK_ERR -1
    #define SOCK_INIT()
    #define SOCK_CLEAN()
#endif

namespace fs = std::filesystem;

ApiHandler::ApiHandler() = default;

std::pair<std::string, int> ApiHandler::handle_api_request(
    const std::string &api_path,
    const std::string &api_root,
    const std::unordered_map<std::string, std::string> &params)
{
    std::string full_path = api_root + "/" + api_path + ".json";
    std::ifstream file(full_path);

    if (!file.is_open())
    {
        return {R"({"error":"Endpoint not found","status":404})", 404};
    }

    std::string json_content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    if (params.empty())
    {
        return {json_content, 200};
    }

    // Buscar el primer objeto que coincida con los parámetros
    size_t pos = 0;
    while ((pos = json_content.find('{', pos)) != std::string::npos)
    {
        size_t start = pos;
        int brace_count = 1;
        pos++;

        while (pos < json_content.size() && brace_count > 0)
        {
            if (json_content[pos] == '{')
                brace_count++;
            else if (json_content[pos] == '}')
                brace_count--;
            pos++;
        }

        if (brace_count != 0)
            break;

        std::string object_str = json_content.substr(start, pos - start);

        bool match = true;
        for (const auto &[key, value] : params)
        {
            if (std::string pattern = "\"" + key + "\": \"" + value + "\""; object_str.find(pattern) == std::string::npos)
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            return {object_str, 200};
        }
    }

    return {R"({"error":"Resource not found","status":404})", 404};
}

// Parsea una query string como ?key=value&other=123
std::unordered_map<std::string, std::string> ApiHandler::parse_query_params(
    const std::string &query)
{
    std::unordered_map<std::string, std::string> params;
    std::istringstream iss(query);
    std::string pair;

    while (std::getline(iss, pair, '&')) {
        auto pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = url_decode(pair.substr(0, pos));
            std::string value = url_decode(pair.substr(pos + 1));
            params[key] = value;
        }
    }

    return params;
}

// Lista archivos .json en el directorio dado
std::vector<std::string> ApiHandler::list_json_files(const std::string &path)
{
    std::vector<std::string> files;

    for (const auto &entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            files.push_back(entry.path().filename().string());
        }
    }

    return files;
}

// bastante heavy esto https://zlib.net/zlib_how.html
std::string ApiHandler::compress_response(const std::string& data) {
    z_stream zs{};
    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        throw std::runtime_error("deflateInit2 failed");

    zs.next_in = (Bytef*)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw std::runtime_error("deflate failed");

    return outstring;
}

std::string ApiHandler::url_decode(const std::string &str)
{
     std::string result;
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (str[i] == '%' && i + 2 < str.size())
            {
                int value;
                std::istringstream iss(str.substr(i + 1, 2));
                if (iss >> std::hex >> value)
                {
                    result += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    result += str[i];
                }
            }
            else if (str[i] == '+')
            {
                result += ' ';
            }
            else
            {
                result += str[i];
            }
        }
        return result;
}

void ApiHandler::setup_dynamic_api_endpoints(const std::string &api_root_path)
{
    Log::info(std::format("Cargando endpoints desde: {}", api_root_path));

    for (const auto &file : fs::directory_iterator(api_root_path)) {
        if (file.path().extension() == ".json") {
            Log::info(std::format("API disponible: {}",  file.path().filename().string()));
        }
    }
}

void ApiHandler::serveApi(socket_t client, const std::string& path, const Config& config, const std::string& clientIP)
{
    std::string api_root = config.get("api_root", "api");
    std::string resource = path.substr(5);
    if (resource.starts_with("/")) resource = resource.substr(1);
    if (resource.empty()) resource = "index";
    std::string json_path = api_root + "/" + resource + ".json";
    Log::info(std::format("{} {} from {}", "GET", path, clientIP));
    std::ifstream file(json_path, std::ios::binary);
    std::string response;

    bool compress = config.get("compress_response", "false") == "true";

    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string body = ss.str();

        if (compress) {
            std::string compressed = compress_response(body);
            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Encoding: gzip\r\n"
                "Vary: Accept-Encoding\r\n"
                "Content-Length: " + std::to_string(compressed.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + compressed;
        } else {
            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: " + std::to_string(body.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" + body;
        }

        send(client, response.c_str(), response.size(), 0);
    } else {
        std::string notFound =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 17\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"error\":404}\n";
        send(client, notFound.c_str(), notFound.size(), 0);
    }
    CLOSESOCKET(client);
}