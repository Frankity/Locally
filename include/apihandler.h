#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#include "config.h"

class ApiHandler
{
public:
    ApiHandler();
    static std::pair<std::string, int> handle_api_request(
        const std::string &api_path,
        const std::string &api_root,
        const std::unordered_map<std::string, std::string> &params = {}
    );

    static void serveApi(SOCKET client, const std::string& path, const Config& config, const std::string& clientIP);

    static std::unordered_map<std::string, std::string> parse_query_params(
        const std::string &query);

    static std::vector<std::string> list_json_files(const std::string &path);

    static std::string url_decode(const std::string &str);

    void setup_dynamic_api_endpoints(const std::string &api_root_path) const;
};

#endif
