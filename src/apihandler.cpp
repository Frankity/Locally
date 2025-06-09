#include "../include/apihandler.h"
#include "../include/log.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <format>
#include <winsock2.h>

namespace fs = std::filesystem;

ApiHandler::ApiHandler() {}

std::pair<std::string, int> ApiHandler::handle_api_request(
    const std::string &api_path,
    const std::string &api_root,
    const std::unordered_map<std::string, std::string> &params)
{
    std::string full_path = api_root + "\\" + api_path + ".json";
    std::ifstream file(full_path);

    if (!file.is_open())
    {
        return {"{\"error\":\"Endpoint not found\",\"status\":404}", 404};
    }

    // Leer archivo completo
    std::string json_content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    if (params.empty())
    {
        return {json_content, 200};
    }

    // Filtrar elementos que coincidan con los parámetros
    std::string result = "[\n";
    size_t pos = 0;
    bool found = false;

    while ((pos = json_content.find('{', pos)) != std::string::npos)
    {
        size_t start = pos;
        int brace_count = 1;
        pos++;

        // Encontrar el final del objeto
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

        // Verificar si el objeto contiene todos los filtros
        bool match = true;
        for (const auto &[key, value] : params)
        {
            std::string pattern = "\"" + key + "\": \"" + value + "\"";
            if (object_str.find(pattern) == std::string::npos)
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            if (found)
                result += ",\n";
            result += object_str;
            found = true;
        }
    }

    result += "\n]";

    if (!found)
    {
        return {"{\"error\":\"Resource not found\",\"status\":404}", 404};
    }

    return {result, 200};
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

// Decodifica un string de URL (por ejemplo: %20 → espacio)
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

void ApiHandler::setup_dynamic_api_endpoints(const std::string &api_root_path) const
{
    Log::info(std::format("Cargando endpoints desde: {}", api_root_path));

    for (const auto &file : fs::directory_iterator(api_root_path)) {
        if (file.path().extension() == ".json") {
            Log::info(std::format("API disponible: {}",  file.path().filename().string()));
        }
    }
}

void ApiHandler::serveApi(SOCKET client, const std::string& path, const Config& config, const std::string& clientIP)
{
    std::string api_root = config.get("api_root", "api");
    std::string resource = path.substr(5);
    if (resource.starts_with("/")) resource = resource.substr(1);
    if (resource.empty()) resource = "index";
    std::string json_path = api_root + "/" + resource + ".json";
    Log::info(std::format("{} {} from {}", "GET", path, clientIP));
    std::ifstream file(json_path, std::ios::binary);
    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string body = ss.str();
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
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
    closesocket(client);
}