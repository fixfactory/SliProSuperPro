#include "Log.h"

#include <vector>
#include <chrono>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

const std::string kLogFileName{ "SliProSuperPro.log" };

LogManager::LogManager()
{
}

LogManager::~LogManager()
{
}

void LogManager::init()
{
    assert(!m_file.is_open());
    m_file.open(kLogFileName, std::ios::out | std::ios::trunc);
    if (!m_file.is_open())
    {
        LOG_ERROR("Failed to open log file %s", kLogFileName);
    }
}

void LogManager::deinit()
{
    if (m_file.is_open())
    {
        m_file.flush();
        m_file.close();
    }
}

LogManager& LogManager::getSingleton()
{
    static LogManager s_singleton;
    return s_singleton;
}

void LogManager::info(const char* format, ...)
{
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);
    std::vector<char> text((__int64)std::vsnprintf(NULL, 0, format, args1) + 1);
    va_end(args1);
    std::vsnprintf(text.data(), text.size(), format, args2);
    va_end(args2);

    std::string timestamp = getTimestamp();
    std::printf("%s %s\n", timestamp.c_str(), text.data());
    if (m_file.is_open())
    {
        m_file << timestamp << ' ' << text.data() << '\n';
        m_file.flush();
    }
}

void LogManager::warn(const char* format, ...)
{
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);
    std::vector<char> text((__int64)std::vsnprintf(NULL, 0, format, args1) + 1);
    va_end(args1);
    std::vsnprintf(text.data(), text.size(), format, args2);
    va_end(args2);

    std::string timestamp = getTimestamp();
    std::printf("%s Warning: %s\n", timestamp.c_str(), text.data());
    if (m_file.is_open())
    {
        m_file << timestamp << " Warning: " << text.data() << '\n';
        m_file.flush();
    }
}

void LogManager::error(const char* format, ...)
{
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);
    std::vector<char> text((__int64)std::vsnprintf(NULL, 0, format, args1) + 1);
    va_end(args1);
    std::vsnprintf(text.data(), text.size(), format, args2);
    va_end(args2);

    std::string timestamp = getTimestamp();
    std::printf("%s Error: %s\n", timestamp.c_str(), text.data());
    if (m_file.is_open())
    {
        m_file << timestamp << " Error: " << text.data() << '\n';
        m_file.flush();
    }
}

void LogManager::error(const std::system_error& error)
{
    LOG_ERROR("%i %s", error.code().value(), error.what());
}

void LogManager::error(const std::exception& exception)
{
    LOG_ERROR("%s", exception.what());
}

std::string LogManager::getTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestamp;
    struct tm buf;
    localtime_s(&buf, &in_time_t);
    timestamp << '[' << std::put_time(&buf, "%T") << ']';
    return timestamp.str();
}
