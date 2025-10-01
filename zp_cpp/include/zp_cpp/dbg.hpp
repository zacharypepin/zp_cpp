#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

#if defined(_WIN32) || defined(_WIN64)

#define _DECORATE_LOG(colour, msg, shouldThrow) \
    do { \
        std::ostringstream oss_MACRO; \
        std::string path_MACRO      = __FILE__; \
        size_t pos_MACRO            = path_MACRO.find_last_of("\\/"); \
        std::string extracted_MACRO = (pos_MACRO == std::string::npos) ? path_MACRO : path_MACRO.substr(pos_MACRO + 1); \
        /* Get current time with milliseconds */ \
        auto now_MACRO              = std::chrono::system_clock::now(); \
        std::time_t t_MACRO         = std::chrono::system_clock::to_time_t(now_MACRO); \
        std::tm tm_MACRO; \
        localtime_s(&tm_MACRO, &t_MACRO); \
        auto ms_MACRO = std::chrono::duration_cast<std::chrono::milliseconds>(now_MACRO.time_since_epoch()) % 1000; \
        oss_MACRO << "\033[33m[" << std::put_time(&tm_MACRO, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms_MACRO.count() << "] \033[34m[" << extracted_MACRO << ":" << __LINE__ << "]" << colour << " " << msg << "\033[0m" << std::endl; \
        std::cout << oss_MACRO.str(); \
    } while (0)

#else

#define _DECORATE_LOG(colour, msg, shouldThrow) \
    do { \
        std::ostringstream oss_MACRO; \
        std::string path_MACRO      = __FILE__; \
        size_t pos_MACRO            = path_MACRO.find_last_of("\\/"); \
        std::string extracted_MACRO = (pos_MACRO == std::string::npos) ? path_MACRO : path_MACRO.substr(pos_MACRO + 1); \
        /* Get current time with milliseconds */ \
        auto now_MACRO              = std::chrono::system_clock::now(); \
        std::time_t t_MACRO         = std::chrono::system_clock::to_time_t(now_MACRO); \
        std::tm tm_MACRO; \
        localtime_r(&t_MACRO, &tm_MACRO); \
        auto ms_MACRO = std::chrono::duration_cast<std::chrono::milliseconds>(now_MACRO.time_since_epoch()) % 1000; \
        oss_MACRO << "\033[33m[" << std::put_time(&tm_MACRO, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms_MACRO.count() << "] \033[34m[" << extracted_MACRO << ":" << __LINE__ << "]" << colour << " " << msg << "\033[0m" << std::endl; \
        std::cout << oss_MACRO.str(); \
    } while (0)

#endif

#define LOG(msg)           _DECORATE_LOG("\033[0m", msg, false)
#define TODO(msg)          _DECORATE_LOG("\033[32m", msg, false)
#define ERR(msg)           _DECORATE_LOG("\033[31m", msg, true)
#define WARN(msg)          _DECORATE_LOG("\033[33m", msg, false)
#define DEPR()             _DECORATE_LOG("\033[31m", "DEPRECATED FUNCTION USED!", true)

#define RGBCOL(x, r, g, b) "\033[38;2;" #r ";" #g ";" #b "m" << x << "\033[0m"

#define RED(x)             "\033[31m" << x << "\033[0m"
#define GREEN(x)           "\033[32m" << x << "\033[0m"
#define ORANGE(x)          "\033[33m" << x << "\033[0m"
#define BLUE(x)            "\033[34m" << x << "\033[0m"
#define MAGENTA(x)         "\033[35m" << x << "\033[0m"
#define CYAN(x)            "\033[36m" << x << "\033[0m"

#define LGREY(x)           RGBCOL(x, 30, 30, 30)
#define DGREY(x)           RGBCOL(x, 10, 10, 10)

#define CLOG(var)          LOG(LGREY(#var ": ") << (var))