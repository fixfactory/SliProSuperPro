#include "Plugin.h"

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
}

void PluginManager::deinit()
{
    TimingManager::getSingleton().unregisterUpdateable(this);
}

void PluginManager::update(timing::seconds deltaTime)
{
}
