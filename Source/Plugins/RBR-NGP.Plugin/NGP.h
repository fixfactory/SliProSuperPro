#pragma once

#include <string>

namespace ngp
{
    constexpr unsigned int kFolderNameCount = 8;
    extern const char *kFolderNames[kFolderNameCount];
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

class NgpManager
{
public:
    static NgpManager &getSingleton();

    NgpManager();
    ~NgpManager();

    void init();
    void deinit();

    bool fetchPhysicsData();

private:
    bool readCommon(unsigned int carIndex, const std::string &gamePath);

    bool m_wasReceivingTelemetry = false;
    ngp::CarPhysics m_carPhysics = {};
};
