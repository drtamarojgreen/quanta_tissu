#ifndef TISSDB_LOG_H
#define TISSDB_LOG_H

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// Log levels
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// This allows controlling the verbosity at compile time.
// For example, a release build could set this to LogLevel::INFO.
#ifndef LOG_LEVEL
#define LOG_LEVEL LogLevel::DEBUG
#endif

inline std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
    }
    return "UNKNOWN";
}

// The core logging macro.
// Using a macro allows us to automatically capture __FILE__ and __LINE__.
#define LOG(level, message) \
    do { \
        if (level >= LOG_LEVEL) { \
            auto now = std::chrono::system_clock::now(); \
            auto in_time_t = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]"; \
            ss << " [" << LogLevelToString(level) << "]"; \
            ss << " [" << __FILE__ << ":" << __LINE__ << "] "; \
            ss << message; \
            std::cerr << ss.str() << std::endl; \
        } \
    } while (0)

// Shorthand macros for convenience.
#define LOG_DEBUG(message)   LOG(LogLevel::DEBUG, message)
#define LOG_INFO(message)    LOG(LogLevel::INFO, message)
#define LOG_WARNING(message) LOG(LogLevel::WARNING, message)
#define LOG_ERROR(message)   LOG(LogLevel::ERROR, message)

#endif // TISSDB_LOG_H
