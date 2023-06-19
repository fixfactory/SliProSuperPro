#pragma once

#include "Timing.h"
#include "PluginInterface.h"

class PhysicsManager : public Updateable
{
public:
    static PhysicsManager &getSingleton();

    PhysicsManager();
    ~PhysicsManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTimeSecs) override;

    bool hasPhysicsData() const;
    const plugin::PhysicsData &getPhysicsData() const;

private:
    bool m_hasPhysicsData = false;
    bool m_wasReceivingTelemetry = false;
    plugin::PhysicsData m_physicsData{};
};