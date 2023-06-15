#pragma once

#include <string>

#include "Timing.h"

namespace ngp
{
constexpr unsigned int kFolderNameCount = 8;
extern const char* kFolderNames[kFolderNameCount];
constexpr unsigned int kGearCount = 8;

struct Drive
{
    int numberOfGears;
};

struct ControlUnit
{
    float gearUpShift[kGearCount];
    float gearDownShift[kGearCount];
    float rpmLimit;
};

struct CarPhysics
{
    Drive drive;
    ControlUnit controlUnit;
};
} // namespace ngp

class NgpManager : public Updateable
{
public:
    static NgpManager& getSingleton();

    NgpManager();
    ~NgpManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTime) override;

private:
    void readCommon(unsigned int carIndex, const std::string& gamePath);

    bool m_wasReceivingTelemetry = false;
    ngp::CarPhysics m_carPhysics = {};
};
