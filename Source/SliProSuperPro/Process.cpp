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

#include <windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx
#include <tlhelp32.h>
#include <locale>
#include <codecvt>

#include "Process.h"
#include "Log.h"
#include "Config.h"
#include "StringHelper.h"
#include "Plugin.h"

const timing::seconds kCheckInterval{ 2.f };

ProcessManager &ProcessManager::getSingleton()
{
    static ProcessManager s_singleton;
    return s_singleton;
}

ProcessManager::ProcessManager()
{
}

ProcessManager::~ProcessManager()
{
}

void ProcessManager::init()
{
    TimingManager::getSingleton().registerUpdateable(this);
}

void ProcessManager::deinit()
{
    m_gamePath.clear();
    PluginManager::getSingleton().setActivePlugin(nullptr);
    TimingManager::getSingleton().unregisterUpdateable(this);
}

void ProcessManager::update(timing::seconds deltaTimeSecs)
{
    // Check only every few seconds to save CPU.
    auto time = std::chrono::steady_clock::now();
    if (time - m_lastCheckTime < kCheckInterval)
    {
        return;
    }

    m_lastCheckTime = time;

    // Check if the currently active game is still running.
    const Plugin *activePlugin = PluginManager::getSingleton().getActivePlugin();
    if (activePlugin != nullptr)
    {
        if (findProcessId(activePlugin->gameExecFileName) != 0)
        {
            // The game is still running.
            return;
        }
        else
        {
            LOG_INFO("Game closed");
            activePlugin->setGameIsRunning(false, "");
            activePlugin = nullptr;
            PluginManager::getSingleton().setActivePlugin(nullptr);
            m_gamePath.clear();
        }
    }

    // Check if any of the supported games is running.
    auto &plugins = PluginManager::getSingleton().getPluginList();
    DWORD pid = 0;
    for (auto &plugin : plugins)
    {
        pid = findProcessId(plugin->gameExecFileName);
        if (pid != 0)
        {
            LOG_INFO("Game running: %s (pid %lu)", plugin->gameExecFileName.c_str(), pid);
            activePlugin = plugin;
            break;
        }
    }

    if (activePlugin == nullptr)
    {
        // None of the supported games is running.
        return;
    }

    m_gamePath = findProcessPath(pid);
    size_t pos = m_gamePath.rfind("\\");
    if (pos != std::string::npos)
    {
        m_gamePath.erase(pos, m_gamePath.length() - pos);
        // gamePath.erase(std::find(gamePath.begin(), gamePath.end(), '\0'), gamePath.end());
    }

    LOG_INFO("Game path: %s", m_gamePath.c_str());
    activePlugin->setGameIsRunning(true, m_gamePath);
    PluginManager::getSingleton().setActivePlugin(activePlugin);
}

const std::string &ProcessManager::getGamePath() const
{
    return m_gamePath;
}

DWORD ProcessManager::findProcessId(const std::string &name) const
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32 info;
    info.dwSize = sizeof(info);
    Process32First(snapshot, &info);
    try
    {
        std::string exeFileName = string::convertFromWide(info.szExeFile);
        if (!name.compare(exeFileName))
        {
            CloseHandle(snapshot);
            return info.th32ProcessID;
        }

        while (Process32Next(snapshot, &info))
        {
            exeFileName = string::convertFromWide(info.szExeFile);
            if (!name.compare(exeFileName))
            {
                CloseHandle(snapshot);
                return info.th32ProcessID;
            }
        }
    }
    catch (const std::exception &exception)
    {
        LOG_ERROR(exception);
    }

    CloseHandle(snapshot);
    return 0;
}

std::string ProcessManager::findProcessPath(DWORD pid) const
{
    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (processHandle == NULL)
    {
        return std::string{};
    }

    wchar_t fileName[MAX_PATH];
    GetModuleFileNameEx(processHandle, NULL, fileName, MAX_PATH);
    CloseHandle(processHandle);

    try
    {
        return string::convertFromWide(fileName);
    }
    catch (const std::exception &exception)
    {
        LOG_ERROR(exception);
    }

    return std::string{};
}
