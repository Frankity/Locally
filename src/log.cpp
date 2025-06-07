#include "../include/log.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <mutex>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

void Log::info(const std::string &msg) { log(LogLevel::INFO, msg); }
void Log::warn(const std::string &msg) { log(LogLevel::WRN, msg); }
void Log::error(const std::string &msg) { log(LogLevel::ERR, msg); }
void Log::debug(const std::string &msg) { log(LogLevel::DBG, msg); }

namespace {
    static std::mutex log_mutex;
}

void Log::log(LogLevel level, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(log_mutex);

    std::string timestamp = get_timestamp();
    std::string levelStr = level_to_string(level);
    std::string color = color_for_level(level);

    std::cout << color << "[" << Log::get_timestamp() << "] [" << levelStr << "] " << msg << COLOR_RESET << std::endl;
}

std::string Log::get_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;

#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Log::level_to_string(LogLevel level)
{
    switch (level)
    {
    case LogLevel::INFO:
        return "INFO ";
    case LogLevel::WRN:
        return "WARN ";
    case LogLevel::ERR:
        return "ERROR ";
    case LogLevel::DBG:
        return "DEBUG ";
    }
    return "UNKNOWN";
}

std::string Log::color_for_level(LogLevel level)
{
    switch (level)
    {
    case LogLevel::INFO:
        return COLOR_GREEN;
    case LogLevel::WRN:
        return COLOR_YELLOW;
    case LogLevel::ERR:
        return COLOR_RED;
    case LogLevel::DBG:
        return COLOR_CYAN;
    }
    return COLOR_WHITE;
}