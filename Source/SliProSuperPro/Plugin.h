#pragma once

#include <string>
#include <vector>

#include "Timing.h"
#include "PluginInterface.h"

typedef int(__stdcall *GetPluginInterfaceVersion)();
typedef int(__stdcall *GetPluginDataVersion)();
typedef void(__stdcall *GetGameExecFileName)(std::string &);
typedef void(__stdcall *SetGameIsRunning)(bool, std::string);
typedef bool(__stdcall *GetTelemetryData)(PluginTelemetryData &);
typedef bool(__stdcall *GetPhysicsData)(PluginPhysicsData &);

struct Plugin
{
    HINSTANCE library{ nullptr };
    std::string path{ };
    int interfaceVersion{ 0 };
    int dataVersion{ 0 };
    std::string gameExecFileName{ };
    SetGameIsRunning setGameIsRunning;
    GetTelemetryData getTelemetryData;
    GetPhysicsData getPhysicsData;
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
