#include "Physics.h"
#include "Telemetry.h"
#include "Plugin.h"

PhysicsManager &PhysicsManager::getSingleton()
{
    static PhysicsManager s_singleton;
    return s_singleton;
}

PhysicsManager::PhysicsManager()
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::init()
{
    memset(&m_physicsData, 0, sizeof(m_physicsData));
    TimingManager::getSingleton().registerUpdateable(this);
}

void PhysicsManager::deinit()
{
    TimingManager::getSingleton().unregisterUpdateable(this);
    m_hasPhysicsData = false;
}

void PhysicsManager::update(timing::seconds deltaTimeSecs)
{
    const Plugin *activePlugin = PluginManager::getSingleton().getActivePlugin();
    if (activePlugin == nullptr)
    {
        return;
    }

    if (!m_wasReceivingTelemetry && TelemetryManager::getSingleton().isReceivingTelemetry())
    {
        m_wasReceivingTelemetry = true;
        m_hasPhysicsData = activePlugin->getPhysicsData(&m_physicsData, sizeof(m_physicsData));
    }
    else if (!TelemetryManager::getSingleton().isReceivingTelemetry())
    {
        m_wasReceivingTelemetry = false;
    }
}

bool PhysicsManager::hasPhysicsData() const
{
    return m_hasPhysicsData;
}

const plugin::PhysicsData &PhysicsManager::getPhysicsData() const
{
    return m_physicsData;
}
