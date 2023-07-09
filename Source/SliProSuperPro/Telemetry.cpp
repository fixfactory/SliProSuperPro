#include "Telemetry.h"
#include "Plugin.h"

TelemetryManager &TelemetryManager::getSingleton()
{
    static TelemetryManager s_singleton;
    return s_singleton;
}

TelemetryManager::TelemetryManager()
{
}

TelemetryManager::~TelemetryManager()
{
}

void TelemetryManager::init()
{
    memset(&m_telemetryData, 0, sizeof(m_telemetryData));
    TimingManager::getSingleton().registerUpdateable(this);
}

void TelemetryManager::deinit()
{
    TimingManager::getSingleton().unregisterUpdateable(this);
    m_receivingTelemetry = false;
}

void TelemetryManager::update(timing::seconds deltaTimeSecs)
{
    const Plugin *activePlugin = PluginManager::getSingleton().getActivePlugin();
    if (activePlugin == nullptr)
    {
        m_receivingTelemetry = false;
        return;
    }

    m_receivingTelemetry = activePlugin->getTelemetryData(&m_telemetryData, sizeof(m_telemetryData));
}

bool TelemetryManager::isReceivingTelemetry() const
{
    return m_receivingTelemetry;
}

const plugin::TelemetryData &TelemetryManager::getTelemetryData() const
{
    return m_telemetryData;
}
