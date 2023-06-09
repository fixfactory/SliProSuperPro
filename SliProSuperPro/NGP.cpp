#include <fstream>
#include <sstream>

#include "NGP.h"
#include "Log.h"
#include "Blackboard.h"

#include "PhysicsNG/rbr.telemetry.data.TelemetryData.h"

namespace ngp
{
    const char* kFolderNames[kFolderNameCount] =
    {
        "c_xsara",
        "h_accent",
        "mg_zr",
        "m_lancer",
        "p_206",
        "s_i2003",
        "t_coroll",
        "s_i2000"
    };
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
    blackboard::carPhysics = &m_carPhysics;
}

void NgpManager::deinit()
{
    blackboard::carPhysics = nullptr;
}

void NgpManager::update(timing::seconds deltaTime)
{
    if (!m_wasReceivingTelemetry
        && blackboard::telemetryData != nullptr
        && !blackboard::gamePath.empty())
    {
        readCommon(blackboard::telemetryData->car_.index_, blackboard::gamePath);
        m_wasReceivingTelemetry = true;
    }
}

void NgpManager::readCommon(unsigned int carIndex, const std::string& gamePath)
{
    memset(&m_carPhysics, 0, sizeof(m_carPhysics));

    if (carIndex >= ngp::kFolderNameCount)
    {
        LOG_ERROR("Invalid car index %ui", carIndex);
        return;
    }

    std::stringstream filePath;
    filePath << gamePath << "Physics\\" << ngp::kFolderNames[carIndex] << "\\common.lsp";
    std::ifstream file(filePath.str());
    if (!file.is_open())
    {
        LOG_ERROR("Could not open file %ls", filePath.str());
        return;
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
}