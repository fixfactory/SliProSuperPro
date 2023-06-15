#pragma once

#include <fstream>
#include <system_error>

#define LOG_INFO(format, ...) LogManager::getSingleton().info(format, ##__VA_ARGS__);

#define LOG_WARN(format, ...) LogManager::getSingleton().warn(format, ##__VA_ARGS__);

#define LOG_ERROR(format, ...) LogManager::getSingleton().error(format, ##__VA_ARGS__);

class LogManager
{
public:
    static LogManager& getSingleton();

    LogManager();
    ~LogManager();

    void init();
    void deinit();

    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    void error(const std::system_error& error);
    void error(const std::exception& exception);

private:
    std::string getTimestamp() const;

    std::ofstream m_file;
};
