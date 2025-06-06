#include "../include/apihandler.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>

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

// Carga endpoints dinámicos (placeholder para lógica futura)
void ApiHandler::setup_dynamic_api_endpoints(const std::string &api_root_path) const
{
    std::cout << "Cargando endpoints desde: " << api_root_path << std::endl;

    for (const auto &file : fs::directory_iterator(api_root_path)) {
        if (file.path().extension() == ".json") {
            std::cout << "-> API disponible: " << file.path().filename().string() << std::endl;
        }
    }
}