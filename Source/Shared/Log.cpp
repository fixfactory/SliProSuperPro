//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2023 Fixfactory
//
// This file is part of SliProSuperPro.
//
// SliProSuperPro is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or any later version.
//
// SliProSuperPro is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with SliProSuperPro. If not, see <http://www.gnu.org/licenses/>.
//

#include <Windows.h>
#include <string>
#include <vector>
#include <chrono>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <sstream>

#include "Log.h"
#include "StringHelper.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

LogManager::LogManager()
{
}

LogManager::~LogManager()
{
}

void LogManager::init()
{
    assert(!m_file.is_open());
    std::string fileName{ getLogFileName() };
    m_file.open(fileName, std::ios::out | std::ios::trunc);
    if (!m_file.is_open())
    {
        LOG_ERROR("Failed to open log file %s", fileName);
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

LogManager &LogManager::getSingleton()
{
    static LogManager s_singleton;
    return s_singleton;
}

void LogManager::info(const char *format, ...)
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

void LogManager::warn(const char *format, ...)
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

void LogManager::error(const char *format, ...)
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

void LogManager::error(const std::system_error &error)
{
    LOG_ERROR("%i %s", error.code().value(), error.what());
}

void LogManager::error(const std::exception &exception)
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

std::string LogManager::getLogFileName() const
{
    wchar_t buf[MAX_PATH];
    GetModuleFileName((HINSTANCE)&__ImageBase, buf, MAX_PATH);
    std::string fileName{ string::convertFromWide(buf) };

    size_t pos = fileName.rfind(".");
    if (pos != std::string::npos)
    {
        fileName.erase(pos, fileName.length() - pos);
    }

    std::stringstream stream;
    stream << fileName << ".log";

    return stream.str();
}
