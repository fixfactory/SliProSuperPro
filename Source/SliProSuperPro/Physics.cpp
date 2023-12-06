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
