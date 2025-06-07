#include <mutex>
#include <vector>
#include <atomic>
#include <thread>
#include <fstream>
#include <sstream>
#include <dirent.h> // Para leer directorios (en Windows usa FindFirstFile/FindNextFile)
#include <iostream>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <format>
#include <unordered_map>


#include "./include/log.h"
#include "./include/config.h"
#include "./include/utils.h"
#include "./include/websocket.h"
#include "./include/filewatcher.h"
#include "./include/apihandler.h"

std::string read_file(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::vector<SOCKET> websocket_clients;
std::mutex clients_mutex;

bool isWebSocketRequest(const std::string &request)
{
    return request.find("Upgrade: websocket") != std::string::npos;
}

std::string get_mime_type(const std::string &path)
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
    std::string ext = path.substr(dot);
    return types.count(ext) ? types[ext] : "application/octet-stream";
}

std::string get_request_path(const std::string &req)
{
    std::istringstream stream(req);
    std::string method, path;
    stream >> method >> path;
    return (path == "/") ? "/index.html" : path;
}

int main()
{

    SetConsoleTitleW(L"Locally - Small Server");

    ApiHandler apiHandler;
    WebSocket webSocket;

    Config config("config.txt");
    if (!config.load())
    {
        Log::warn("Error cargando configuración, usando valores por defecto");
    }

    bool enable_live_reload = config.getBool("live_reload", true); // Valor por defecto true

    std::string doc_root = config.get("document_root", "public");
    FileWatcher watcher;
    watcher.setWatchPath(doc_root);

    std::atomic<bool> changes_detected(false);
    if (enable_live_reload)
    {
        std::thread watcher_thread([&]()
                                   {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (watcher.checkChanges()) {
                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    for (auto it = websocket_clients.begin(); it != websocket_clients.end();) {
                        SOCKET ws_client = *it;
                        
                        if (!webSocket.sendWebSocketMessage(ws_client, "reload")) {
                            closesocket(ws_client);
                            it = websocket_clients.erase(it);
                        } else {
                            ++it;
                        }
                    }
                }
                watcher.setWatchPath(doc_root);
            }
        } });
        watcher_thread.detach();
        Log::info("Live reload habilitado");
    }
    else
    {
        Log::info("Live reload deshabilitado por configuracion");
    }

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    int port_int = config.getInt("port", 8080);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_int));
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (sockaddr *)&addr, sizeof(addr));
    listen(server, SOMAXCONN);

    Log::debug(std::format("Servidor activo en http://localhost:{}", std::to_string(port_int)));

    std::string api_root_path = config.get("api_root", "api_resources");
    apiHandler.setup_dynamic_api_endpoints(api_root_path);

    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET client = accept(server, (sockaddr *)&clientAddr, &clientSize);
        if (client == INVALID_SOCKET)
            continue;

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientAddr.sin_port);

        char buffer[4096] = {0};
        int bytesReceived = recv(client, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            closesocket(client);
            continue;
        }

        std::string request(buffer);
        std::string path = get_request_path(request);
        std::string full_path = config.get("document_root", "public") + path;

        std::string body = read_file(full_path);
        std::string response;

        std::istringstream iss(request);
        std::string method;
        iss >> method;

        if (path.find("/api/") == 0)
        {
            std::string api_endpoint = path.substr(5); // Elimina "/api/"

            // Procesar parámetros de consulta
            std::unordered_map<std::string, std::string> params;
            size_t query_pos = api_endpoint.find('?');
            if (query_pos != std::string::npos)
            {
                std::string query_str = api_endpoint.substr(query_pos + 1);
                api_endpoint = api_endpoint.substr(0, query_pos);
                params = ApiHandler::parse_query_params(query_str);
            }

            auto [json_response, status_code] = ApiHandler::handle_api_request(api_endpoint, api_root_path, params);

            response = "HTTP/1.1 " + std::to_string(status_code) +
                       (status_code == 200 ? " OK" : " Not Found") + "\r\n"
                                                                     "Content-Type: application/json\r\n"
                                                                     "Access-Control-Allow-Origin: *\r\n"
                                                                     "Content-Length: " +
                       std::to_string(json_response.size()) + "\r\n"
                                                              "\r\n" +
                       json_response;

            send(client, response.c_str(), response.size(), 0);
            closesocket(client);
            continue;
        }

        if (enable_live_reload && request.find("Upgrade: websocket") != std::string::npos)
        {
            std::string key = WebSocket::getWebSocketKey(request);
            if (!key.empty())
            {
                std::string accept_key = WebSocket::generateWebSocketAccept(key);
                std::string response =
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: " +
                    accept_key + "\r\n"
                                 "Sec-WebSocket-Version: 13\r\n"
                                 "\r\n";

                if (send(client, response.c_str(), response.size(), 0) > 0)
                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    websocket_clients.push_back(client);
                    continue;
                }

                continue;
            }
        }

        else
        {

            if (!body.empty())
            {

                std::string content_type = get_mime_type(full_path);

                if (enable_live_reload && content_type == "text/html")
                {
                    size_t body_pos = body.find("</body>");
                    if (body_pos != std::string::npos)
                    {
                        body.insert(body_pos, "<script src='/live-reload.js'></script>");
                    }
                }

                response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: " +
                    content_type + "\r\n"
                                   "Content-Length: " +
                    std::to_string(body.size()) + "\r\n"
                                                  "Connection: close\r\n"
                                                  "Server: " +
                    config.get("server_name", "kity/1.0\r\n") + "\r\n"
                                                                "X-Powered-By: Frankity\r\n"
                                                                "\r\n" +
                    body;
                Log::info(method + " " + path + " from " + clientIP + ":" + std::to_string(clientPort));
            }
            else
            {
                std::string not_found = "<h1>404 - Not Found</h1>";
                response =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " +
                    std::to_string(not_found.size()) + "\r\n"
                                                       "Connection: close\r\n"
                                                       "Server: kity/1.0\r\n"
                                                       "X-Powered-By: Frankity\r\n"
                                                       "\r\n" +
                    not_found;
                Log::error(method + " " + path + " from " + clientIP + ":" + std::to_string(clientPort));
            }
        }

        send(client, response.c_str(), response.size(), 0);
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
