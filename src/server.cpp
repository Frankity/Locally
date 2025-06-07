
#include "../include/server.h"
#include "../include/httpfilehandler.h"
#include "../include/log.h"
#include <winsock2.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <fstream>
#include <ws2tcpip.h>
#include <format>

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
    if (serverSocket == INVALID_SOCKET)
    {
        Log::error("Error creating socket");
        WSACleanup();
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
        WSACleanup();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
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

    while (true)
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

    HttpFileHandler::serveFile(client, full_path, config, clientIP);
}