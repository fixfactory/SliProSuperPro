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

#include <fstream>
#include <sstream>

#include "NGP.h"
#include "Log.h"
#include "Telemetry.h"

#include "PhysicsNG/rbr.telemetry.data.TelemetryData.h"

namespace ngp
{
    const char *kFolderNames[kFolderNameCount] = { "c_xsara", "h_accent", "mg_zr",    "m_lancer",
                                                   "p_206",   "s_i2003",  "t_coroll", "s_i2000" };
}

NgpManager &NgpManager::getSingleton()
{
    static NgpManager s_singleton;
    return s_singleton;
}

NgpManager::NgpManager()
{
}

NgpManager::~NgpManager()
{
}

void NgpManager::init()
{
    memset(&m_carPhysics.controlUnit, 0, sizeof(m_carPhysics.controlUnit));
}

void NgpManager::deinit()
{
}

bool NgpManager::fetchPhysicsData(const std::string &gamePath)
{
    if (TelemetryManager::getSingleton().isReceivingTelemetry() && !gamePath.empty())
    {
        auto carIndex = TelemetryManager::getSingleton().getRBRTelemetryData().car_.index_;
        if (readCommon(carIndex, gamePath))
        {
            m_physicsData.gearCount = m_carPhysics.drive.numberOfGears;
            m_physicsData.rpmLimit = m_carPhysics.controlUnit.rpmLimit;

            static_assert(sizeof(m_physicsData.rpmDownshift) >= sizeof(m_carPhysics.controlUnit.gearDownShift));
            static_assert(sizeof(m_physicsData.rpmUpshift) >= sizeof(m_carPhysics.controlUnit.gearUpShift));

            memcpy(m_physicsData.rpmDownshift, m_carPhysics.controlUnit.gearDownShift,
                   sizeof(m_carPhysics.controlUnit.gearDownShift));

            memcpy(m_physicsData.rpmUpshift, m_carPhysics.controlUnit.gearUpShift,
                   sizeof(m_carPhysics.controlUnit.gearUpShift));

            return true;
        }
    }
    return false;
}

const plugin::PhysicsData &NgpManager::getPhysicsData() const
{
    return m_physicsData;
}

bool NgpManager::readCommon(unsigned int carIndex, const std::string &gamePath)
{
    LOG_INFO("Reading physics file for car index %u", carIndex);
    memset(&m_carPhysics, 0, sizeof(m_carPhysics));

    if (carIndex >= ngp::kFolderNameCount)
    {
        LOG_ERROR("Invalid car index %u", carIndex);
        return false;
    }

    std::stringstream filePath;
    filePath << gamePath << "\\Physics\\" << ngp::kFolderNames[carIndex] << "\\common.lsp";
    std::ifstream file(filePath.str());
    if (!file.is_open())
    {
        LOG_ERROR("Could not open file %ls", filePath.str());
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        std::string name;
        float value;
        if (stream >> name >> value)
        {
            if (name.find("NumberOfGears") != std::string::npos)
            {
                m_carPhysics.drive.numberOfGears = (int)value;
            }
            else if (name.find("Gear") != std::string::npos)
            {
                if (name.find("Upshift") != std::string::npos)
                {
                    int gear = atoi(&name[4]);
                    if (gear < 0 || gear >= ngp::kGearCount)
                    {
                        LOG_WARN("Failed parsing upshift gear. %i", gear);
                        continue;
                    }
                    m_carPhysics.controlUnit.gearUpShift[gear] = value;
                }
                else if (name.find("Downshift") != std::string::npos)
                {
                    int gear = atoi(&name[4]);
                    if (gear < 0 || gear >= ngp::kGearCount)
                    {
                        LOG_WARN("Failed parsing downshift gear. %i", gear);
                        continue;
                    }
                    m_carPhysics.controlUnit.gearDownShift[gear] = value;
                }
            }
            else if (name.find("RPMLimit") != std::string::npos)
            {
                m_carPhysics.controlUnit.rpmLimit = value;
            }
        }
    }

    return true;
}
