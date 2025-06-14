#include <httpfilehandler.h>
#include <log.h>
#include <winsock2.h>
#include <fstream>
#include <sstream>
#include <format>

std::string getMimeType(const std::string &path)
{
    static std::unordered_map<std::string, std::string> types = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".txt", "text/plain"}};

    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    const std::string ext = path.substr(dot);
    return types.contains(ext) ? types[ext] : "application/octet-stream";
}

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void HttpFileHandler::serveFile(const SOCKET client, const std::string& path, const Config& config, const std::string& clientIP) {
    const std::string body = readFile(path);
    std::string response;
    std::string method = "GET";

    if (!body.empty()) {
        const std::string content_type = getMimeType(path);
        response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: " + content_type + "\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "Server: " + config.get("server_name", "Locally/1.0") + "\r\n"
            "X-Powered-By: Frankity\r\n"
            "\r\n" + body;

        Log::info(std::format("{} {} from {}", method, path, clientIP));
        send(client, response.c_str(), response.size(), 0);
    } else {
        const std::string notFound =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "Connection: close\r\n"
            "\r\n"
            "404 Not Found";
        send(client, notFound.c_str(), notFound.size(), 0);
    }
    closesocket(client);
}