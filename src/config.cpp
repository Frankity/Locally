#include <config.h>
#include <log.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <format>
#include <utility>

Config::Config(std::string filename) : filename_(std::move(filename)) {}

bool Config::load()
{
    std::ifstream file(filename_);
    if (!file.is_open())
    {
        Log::warn(std::format("Config file could not be loaded: {}", filename_ ));
        return false;
    }

    file.seekg(0, std::ios::end);

    if (std::streampos file_size = file.tellg(); file_size == 0)
    {
        Log::warn(std::format("Config file is empty: {}", filename_));
        return false;
    }
    
    file.seekg(0, std::ios::beg);

    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line))
    {
        line_number++;
        line = trim(line);
        
        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos)
        {
            Log::warn(std::format("Line {} is wrongly formatted in {} at line {}", line_number, filename_, line));
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        const std::string value = trim(line.substr(pos + 1));

        if (key.empty())
        {
            std::cerr << "Clave vacía en línea " << line_number << " de " << filename_ << std::endl;
            continue;
        }

        values_[key] = value;
    }

    file.close();
    Log::info(std::format("Config loaded: {} parametters from {}", values_.size(), filename_));
    return true;
}

std::string Config::get(const std::string &key, const std::string &defaultValue) const
{
    if (const auto it = values_.find(key); it != values_.end())
    {
        return it->second;
    }
    return defaultValue;
}

int Config::getInt(const std::string &key, const int defaultValue) const
{
    std::string value = get(key);
    if (value.empty())
        return defaultValue;
    
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        Log::warn(std::format("Error transforming '{}' to int '{}'", value, key));
        return defaultValue;
    }
}

bool Config::getBool(const std::string &key, const bool defaultValue) const
{
    const std::string value = get(key);
    if (value.empty())
        return defaultValue;
    
    // Convertir a minúsculas para comparación
    std::string lower_value = value;
    std::ranges::transform(lower_value, lower_value.begin(), ::tolower);
    
    return (lower_value == "true" || lower_value == "1" || lower_value == "yes" || lower_value == "on");
}

bool Config::hasKey(const std::string &key) const
{
    return values_.contains(key);
}

void Config::printAll() const
{
    Log::info(std::format("Config loaded from {}", filename_));
    for (const auto&[fst, snd] : values_)
    {
        std::cout << "  " << fst << " = " << snd << std::endl;
    }
}

std::string Config::trim(const std::string& str) const
{
    const size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";

    const size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
