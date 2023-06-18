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
        plugin->path = entry.path().string();

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

        GetPluginDataVersion getPluginDataVersion =
            (GetPluginDataVersion)GetProcAddress(plugin->library, "getPluginDataVersion");

        if (!getPluginDataVersion)
        {
            LOG_ERROR("Could not locate the function getPluginDataVersion()");
            FreeLibrary(plugin->library);
            delete plugin;
            continue;
        }

        plugin->dataVersion = getPluginDataVersion();

        m_plugins.push_back(plugin);

        LOG_INFO("Plugin interface version: %i", plugin->interfaceVersion);
        LOG_INFO("Plugin data version: %i", plugin->dataVersion);
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
