#pragma once
#include <string>
#include <winsock2.h>
#include "config.h"

class HttpFileHandler
{
public:
    static void serveFile(SOCKET client, const std::string &path, const Config &config, const std::string &clientIP);
};