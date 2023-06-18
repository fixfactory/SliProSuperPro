#pragma once

#include <string>
#include <vector>

#include "Timing.h"

typedef int(__stdcall *GetPluginInterfaceVersion)();
typedef int(__stdcall *GetPluginDataVersion)();

struct Plugin
{
    HINSTANCE library{ nullptr };
    std::string path{ };
    int interfaceVersion{ 0 };
    int dataVersion{ 0 };
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
    void loadPlugins();
    void unloadPlugins();

    std::vector<Plugin *> m_plugins;
};
