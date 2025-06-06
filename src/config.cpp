#include "../include/config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

Config::Config(const std::string &filename) : filename_(filename) {}

bool Config::load()
{
    std::ifstream file(filename_);
    if (!file.is_open())
    {
        std::cerr << "No se pudo abrir el archivo de configuración: " << filename_ << std::endl;
        return false;
    }

    // Verificar si el archivo tiene contenido
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    
    if (file_size == 0)
    {
        std::cerr << "El archivo de configuración está vacío: " << filename_ << std::endl;
        return false;
    }
    
    // Volver al inicio del archivo
    file.seekg(0, std::ios::beg);

    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line))
    {
        line_number++;
        line = trim(line);
        
        // Ignorar líneas vacías y comentarios
        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos)
        {
            std::cerr << "Línea " << line_number << " mal formateada en " << filename_ 
                      << ": " << line << std::endl;
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (key.empty())
        {
            std::cerr << "Clave vacía en línea " << line_number << " de " << filename_ << std::endl;
            continue;
        }

        values_[key] = value;
    }

    file.close();
    std::cout << "Configuración cargada: " << values_.size() << " parámetros desde " << filename_ << std::endl;
    return true;
}

std::string Config::get(const std::string &key, const std::string &defaultValue) const
{
    auto it = values_.find(key);
    if (it != values_.end())
    {
        return it->second;
    }
    return defaultValue;
}

int Config::getInt(const std::string &key, int defaultValue) const
{
    std::string value = get(key);
    if (value.empty())
        return defaultValue;
    
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        std::cerr << "Error convirtiendo '" << value << "' a entero para clave '" << key << "'" << std::endl;
        return defaultValue;
    }
}

bool Config::getBool(const std::string &key, bool defaultValue) const
{
    std::string value = get(key);
    if (value.empty())
        return defaultValue;
    
    // Convertir a minúsculas para comparación
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    
    return (lower_value == "true" || lower_value == "1" || lower_value == "yes" || lower_value == "on");
}

bool Config::hasKey(const std::string &key) const
{
    return values_.find(key) != values_.end();
}

void Config::printAll() const
{
    std::cout << "Configuración cargada desde " << filename_ << ":" << std::endl;
    for (const auto& pair : values_)
    {
        std::cout << "  " << pair.first << " = " << pair.second << std::endl;
    }
}

std::string Config::trim(const std::string& str) const
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
