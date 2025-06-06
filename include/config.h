#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

class Config
{
private:
    std::string filename_;
    std::unordered_map<std::string, std::string> values_;
    
    std::string trim(const std::string& str) const;

public:
    Config(const std::string &filename);
    bool load();
    std::string get(const std::string &key, const std::string &defaultValue = "") const;
    
    int getInt(const std::string &key, int defaultValue = 0) const;
    bool getBool(const std::string &key, bool defaultValue = false) const;
    bool hasKey(const std::string &key) const;
    void printAll() const;
};

#endif