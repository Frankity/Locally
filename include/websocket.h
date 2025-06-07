#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <string>
#include <winsock2.h>

class WebSocket
{
public:
    WebSocket();

    bool sendWebSocketMessage(SOCKET client, const std::string &message);
    static std::string getWebSocketKey(const std::string &request);
    static std::string generateWebSocketAccept(const std::string &key);
};

#endif