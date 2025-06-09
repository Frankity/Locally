#include "../include/server.h"
#include "../include/httpfilehandler.h"
#include "../include/log.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <fstream>
#include <format>

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

Server::Server(const std::string &configPath)
    : config(configPath)
{

    if (!config.load())
    {
        Log::warn("Error loading config, using default values");
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        Log::error("WSAStartup failed");
        exit(1);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == SOCK_ERR)
    {
        Log::error("Error creating socket");
        SOCK_CLEAN();
        exit(1);
    }

    int port_int = config.getInt("port", 8080);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port_int));

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        Log::error("Error binding socket");
        closesocket(serverSocket);
        SOCK_CLEAN();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        CLOSESOCKET(serverSocket);
        SOCK_CLEAN();
        exit(1);
    }
}

void Server::run()
{
    acceptConnections();
}

void Server::acceptConnections()
{
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    std::atomic<bool> running{true};

    while (running)
    {
        SOCKET client = accept(serverSocket, (sockaddr *)&clientAddr, &addrLen);
        if (client == INVALID_SOCKET)
        {
            Log::error("Error accepting new client connection");
            continue;
        }
        std::thread(&Server::handleClient, this, client, clientAddr).detach();
    }
}

std::string Server::getRequestPath(const std::string &req)
{
    std::istringstream stream(req);
    std::string method, path;
    stream >> method >> path;
    return (path == "/") ? "/index.html" : path;
}

std::string Server::readFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void Server::handleClient(SOCKET client, sockaddr_in clientAddr)
{

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

    char buffer[4096] = {0};
    int bytesReceived = recv(client, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        closesocket(client);
        return;
    }
    std::string request(buffer);
    std::string path = Server::getRequestPath(request);
    std::string full_path = config.get("document_root", "public") + path;

    if (path.starts_with("/api/")) {
        ApiHandler::serveApi(client, path, config, clientIP);
        return;
    }else {
        HttpFileHandler::serveFile(client, full_path, config, clientIP);
    }

}