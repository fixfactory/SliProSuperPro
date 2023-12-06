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

#pragma once

#include <fstream>
#include <system_error>

#define LOG_INFO(format, ...) LogManager::getSingleton().info(format, ##__VA_ARGS__);

#define LOG_WARN(format, ...) LogManager::getSingleton().warn(format, ##__VA_ARGS__);

#define LOG_ERROR(format, ...) LogManager::getSingleton().error(format, ##__VA_ARGS__);

class LogManager
{
public:
    static LogManager &getSingleton();

    LogManager();
    ~LogManager();

    void init();
    void deinit();

    void info(const char *format, ...);
    void warn(const char *format, ...);
    void error(const char *format, ...);
    void error(const std::system_error &error);
    void error(const std::exception &exception);

private:
    std::string getTimestamp() const;
    std::string getLogFileName() const;

    std::ofstream m_file;
};
