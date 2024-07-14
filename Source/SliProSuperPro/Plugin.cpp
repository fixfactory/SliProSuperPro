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
#include <filesystem>

#include "Plugin.h"
#include "Log.h"
#include "StringHelper.h"

PluginManager &PluginManager::getSingleton()
{
    static PluginManager s_singleton;
    return s_singleton;
}

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

void PluginManager::init()
{
    TimingManager::getSingleton().registerUpdateable(this);
    loadPlugins();
}

void PluginManager::deinit()
{
    unloadPlugins();
    TimingManager::getSingleton().unregisterUpdateable(this);
}

void PluginManager::update(timing::seconds deltaTimeSecs)
{
}

const PluginManager::PluginList &PluginManager::getPluginList()
{
    return m_plugins;
}

void PluginManager::setActivePlugin(const Plugin *plugin)
{
    m_activePlugin = plugin;
}

const Plugin *PluginManager::getActivePlugin() const
{
    return m_activePlugin;
}

void PluginManager::loadPlugins()
{
    // Get the path of our executable. We'll search for plugin DLLs in the same directory.
    wchar_t buf[MAX_PATH];
    GetModuleFileName(nullptr, buf, MAX_PATH);
    std::string execPath{ string::convertFromWide(buf) };

    size_t pos = execPath.rfind("\\");
    if (pos != std::string::npos)
    {
        execPath.erase(pos, execPath.length() - pos);
    }

    // Iterate over all DLLs matching our naming convention for plugins.
    for (const auto &entry : std::filesystem::directory_iterator(execPath))
    {
        if (!entry.is_regular_file())
            continue;

        size_t pos = entry.path().string().rfind(".Plugin.dll");
        if (pos == std::string::npos)
            continue;

        Plugin *plugin = new Plugin();
        plugin->libraryPath = entry.path().string();

        std::wstring path{ string::convertToWide(entry.path().string().c_str()) };
        plugin->library = LoadLibrary(path.c_str());
        if (!plugin->library)
        {
            LOG_ERROR("Could not load the library %s", entry.path().filename().string().c_str());
            delete plugin;
            continue;
        }

        GetPluginInterfaceVersion getPluginInterfaceVersion =
            (GetPluginInterfaceVersion)GetProcAddress(plugin->library, "getPluginInterfaceVersion");

        if (!getPluginInterfaceVersion)
        {
            LOG_ERROR("Could not locate the function getPluginInterfaceVersion()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        plugin->interfaceVersion = getPluginInterfaceVersion();
        LOG_INFO("Plugin interface version: %i", plugin->interfaceVersion);

        SupportsInterfaceVersion supportsInterfaceVersion =
            (SupportsInterfaceVersion)GetProcAddress(plugin->library, "supportsInterfaceVersion");

        if (!supportsInterfaceVersion)
        {
            LOG_ERROR("Could not locate the function supportsInterfaceVersion()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        if (!supportsInterfaceVersion(plugin::kInterfaceVersion))
        {
            LOG_ERROR("Plugin does not support our interface version %i", plugin::kInterfaceVersion);
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        GetGameExecFileName getGameExecFileName =
            (GetGameExecFileName)GetProcAddress(plugin->library, "getGameExecFileName");

        if (!getGameExecFileName)
        {
            LOG_ERROR("Could not locate the function getGameExecFileName()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        getGameExecFileName(plugin->gameExecFileName);

        plugin->setGameIsRunning = (SetGameIsRunning)GetProcAddress(plugin->library, "setGameIsRunning");

        if (!plugin->setGameIsRunning)
        {
            LOG_ERROR("Could not locate the function setGameIsRunning()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        plugin->getTelemetryData = (GetTelemetryData)GetProcAddress(plugin->library, "getTelemetryData");

        if (!plugin->getTelemetryData)
        {
            LOG_ERROR("Could not locate the function getTelemetryData()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        plugin->getPhysicsData = (GetPhysicsData)GetProcAddress(plugin->library, "getPhysicsData");

        if (!plugin->getPhysicsData)
        {
            LOG_ERROR("Could not locate the function getPhysicsData()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        plugin->getPhysicsDataEveryFrame = (GetPhysicsDataEveryFrame)GetProcAddress(plugin->library, "getPhysicsDataEveryFrame");

        if (!plugin->getPhysicsDataEveryFrame)
        {
            LOG_ERROR("Could not locate the function getPhysicsDataEveryFrame()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        m_plugins.push_back(plugin);
    }
}

void PluginManager::unloadPlugins()
{
    for (auto &plugin : m_plugins)
    {
        if (plugin->library != nullptr)
        {
            FreeLibrary(plugin->library);
        }

        delete plugin;
    }
    m_plugins.clear();
}
