// projects/libs/BasicLogger/include/BasicLogger.h
// ================================================
// Minimal header-only logger with console and file support
//
// Version: 1.1.0
// Date:    2025-12-09
// Status:  Stable
// License: MIT

#ifndef BASIC_LOGGER_H
#define BASIC_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>

namespace BasicLogger {

    // Log levels
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    // Convert level to string
    inline const char* levelToString(Level level) {
        switch (level) {
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
        default:             return "UNKNOWN";
        }
    }

    // Get current timestamp as string
    inline std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::tm tm_info{};
#ifdef _WIN32
        localtime_s(&tm_info, &time);
#else
        localtime_r(&time, &tm_info);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    // Logger class
    class Logger {
    public:
        // Constructor - console only
        Logger()
            : m_minLevel(Level::Info),
            m_logToConsole(true) {
        }

        // Constructor - with log file
        explicit Logger(const std::string& logFilePath)
            : m_minLevel(Level::Info),
            m_logToConsole(true) {
            setLogFile(logFilePath);
        }

        // Set minimum log level
        void setLevel(Level level) {
            m_minLevel = level;
        }

        // Enable/disable console output
        void setConsoleOutput(bool enabled) {
            m_logToConsole = enabled;
        }

        // Set log file (enables file logging)
        bool setLogFile(const std::string& path) {
            m_logFile = std::make_unique<std::ofstream>(path, std::ios::app);
            return m_logFile && m_logFile->is_open();
        }

        // Close log file
        void closeLogFile() {
            if (m_logFile) {
                m_logFile->close();
                m_logFile.reset();
            }
        }

        // Log message
        void log(Level level, const std::string& message) {
            if (level < m_minLevel) {
                return;
            }

            std::ostringstream formatted;
            formatted << "[" << getTimestamp() << "] "
                << "[" << levelToString(level) << "] "
                << message;

            const std::string output = formatted.str();

            if (m_logToConsole) {
                std::cout << output << std::endl;
            }

            if (m_logFile && m_logFile->is_open()) {
                *m_logFile << output << std::endl;
                m_logFile->flush();
            }
        }

        // Convenience methods
        void debug(const std::string& msg) { log(Level::Debug, msg); }
        void info(const std::string& msg) { log(Level::Info, msg); }
        void warning(const std::string& msg) { log(Level::Warning, msg); }
        void error(const std::string& msg) { log(Level::Error, msg); }

    private:
        Level m_minLevel;
        bool m_logToConsole;
        std::unique_ptr<std::ofstream> m_logFile;
    };

    // Global logger instance (optional convenience)
    inline Logger& getGlobalLogger() {
        static Logger instance;
        return instance;
    }

    // Global convenience functions
    inline void setLogLevel(Level level) {
        getGlobalLogger().setLevel(level);
    }

    inline void setLogFile(const std::string& path) {
        getGlobalLogger().setLogFile(path);
    }

    inline void setConsoleOutput(bool enabled) {
        getGlobalLogger().setConsoleOutput(enabled);
    }

    inline void closeLogFile() {
        getGlobalLogger().closeLogFile();
    }

    inline void logDebug(const std::string& msg) {
        getGlobalLogger().debug(msg);
    }

    inline void logInfo(const std::string& msg) {
        getGlobalLogger().info(msg);
    }

    inline void logWarning(const std::string& msg) {
        getGlobalLogger().warning(msg);
    }

    inline void logError(const std::string& msg) {
        getGlobalLogger().error(msg);
    }

} // namespace BasicLogger

#endif // BASIC_LOGGER_H
