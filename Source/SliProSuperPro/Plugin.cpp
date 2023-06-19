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

void PluginManager::update(timing::seconds deltaTime)
{
    for (auto &plugin : m_plugins)
    {
        
    }
}

const PluginManager::PluginList &PluginManager::getPluginList()
{
    return m_plugins;
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

        plugin->fetchTelemetryData = (FetchTelemetryData)GetProcAddress(plugin->library, "fetchTelemetryData");

        if (!plugin->fetchTelemetryData)
        {
            LOG_ERROR("Could not locate the function fetchTelemetryData()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        plugin->fetchPhysicsData = (FetchPhysicsData)GetProcAddress(plugin->library, "fetchPhysicsData");

        if (!plugin->fetchPhysicsData)
        {
            LOG_ERROR("Could not locate the function fetchPhysicsData()");
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

        m_plugins.push_back(plugin);

        LOG_INFO("Plugin interface version: %i", plugin->interfaceVersion);
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
