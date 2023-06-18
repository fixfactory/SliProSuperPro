#pragma once

#include <string>
#include <vector>

#include "Timing.h"

typedef int(__stdcall *GetPluginVersion)();

struct Plugin
{
    std::string execName = { };
};

class PluginManager : public Updateable
{
public:
    static PluginManager &getSingleton();

    PluginManager();
    ~PluginManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTime) override;

 private:
    void findPlugins();

    std::vector<Plugin> m_plugins;
};
