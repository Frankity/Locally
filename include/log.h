#ifndef LOG_H
#define LOG_H

#pragma once
#include <string>

enum class LogLevel
{
    INFO,
    WRN,
    ERR,
    DBG
};

class Log
{
public:
    static void info(const std::string &msg);
    static void warn(const std::string &msg);
    static void error(const std::string &msg);
    static void debug(const std::string &msg);

private:
    static void log(LogLevel level, const std::string &msg);
    static std::string get_timestamp();
    static std::string level_to_string(LogLevel level);
    static std::string color_for_level(LogLevel level);
};

#endif