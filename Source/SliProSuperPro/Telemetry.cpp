//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2023 Fixfactory
//
// This file is part of SliProSuperPro.
//
// SliProSuperPro is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or any later version.
//
// SliProSuperPro is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with SliProSuperPro. If not, see <http://www.gnu.org/licenses/>.
//

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
