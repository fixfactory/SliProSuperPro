#include <Windows.h>

#include "Plugin.h"
#include "Log.h"

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
    findPlugins();
}

void PluginManager::deinit()
{
    TimingManager::getSingleton().unregisterUpdateable(this);
}

void PluginManager::update(timing::seconds deltaTime)
{
}

void PluginManager::findPlugins()
{
    HINSTANCE library = LoadLibrary(L"E:\\Dev\\sli-pro-super-pro\\SliProSuperPro\\Bin\\x64\\Debug\\iRacing.Plugin.dll");
    if (!library)
    {
        LOG_ERROR("Could not load the dynamic library");
        return;
    }

    GetPluginVersion getPluginVersion = (GetPluginVersion)GetProcAddress(library, "getPluginVersion");
    if (!getPluginVersion)
    {
        LOG_ERROR("Could not locate the function");
        FreeLibrary(library);
        return;
    }

    int pluginVersion = getPluginVersion();
    LOG_INFO("Plugin version: %i", pluginVersion);
    FreeLibrary(library);
}
