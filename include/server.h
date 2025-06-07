#pragma once
#include "config.h"
#include "apihandler.h"
#include "websocket.h"
#include "filewatcher.h"
#include <mutex>
#include <vector>

class Server
{
public:
    Server(const std::string &configPath);
    void run();

private:
    void acceptConnections();
    void handleClient(SOCKET client, sockaddr_in clientAddr);
    std::string getRequestPath(const std::string &req);
    std::string readFile(const std::string &path);
    std::string getMimeType(const std::string &path);

    Config config;
    ApiHandler apiHandler;
    WebSocket webSocket;
    FileWatcher fileWatcher;
    SOCKET serverSocket;
    std::mutex client_mutex;
    std::vector<SOCKET> ws_clients;

};