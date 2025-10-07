#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <filesystem>

// =========================================================================================================================================
// =========================================================================================================================================
// =========================================================================================================================================
// =========================================================================================================================================
namespace zp::log
{
    struct Logger
    {
        enum Level
        {
            INFO  = 0,
            WARN  = 1,
            ERROR = 2
        };

        std::filesystem::path file_path;
        std::ofstream file_stream;
        std::mutex log_mutex;
        bool initialised;
    };

    void init(Logger* logger, const std::filesystem::path& file_path);
    void cleanup(Logger* logger);
    void log(Logger* logger, Logger::Level level, const std::string& message);
}

// =========================================================================================================================================
// =========================================================================================================================================
// =========================================================================================================================================
// =========================================================================================================================================
#define ZP_LOG_INFO(logger, msg) \
    do { \
        if ((logger) && (logger)->initialised) \
        { \
            std::ostringstream oss; \
            std::string path     = __FILE__; \
            size_t pos           = path.find_last_of("\\/"); \
            std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1); \
            oss << "[" << filename << ":" << __LINE__ << "] " << msg; \
            zp::log::log(logger, zp::log::Logger::INFO, oss.str()); \
        } \
    } while (0)

#define ZP_LOG_WARN(logger, msg) \
    do { \
        if ((logger) && (logger)->initialised) \
        { \
            std::ostringstream oss; \
            std::string path     = __FILE__; \
            size_t pos           = path.find_last_of("\\/"); \
            std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1); \
            oss << "[" << filename << ":" << __LINE__ << "] " << msg; \
            zp::log::log(logger, zp::log::Logger::WARN, oss.str()); \
        } \
    } while (0)

#define ZP_LOG_ERROR(logger, msg) \
    do { \
        if ((logger) && (logger)->initialised) \
        { \
            std::ostringstream oss; \
            std::string path     = __FILE__; \
            size_t pos           = path.find_last_of("\\/"); \
            std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1); \
            oss << "[" << filename << ":" << __LINE__ << "] " << msg; \
            zp::log::log(logger, zp::log::Logger::ERROR, oss.str()); \
        } \
    } while (0)
