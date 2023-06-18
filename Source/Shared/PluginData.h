#include <map>

constexpr int kPluginInterfaceVersion = 1;
constexpr int kPluginDataVersion = 1;

enum class kPluginTelemetryField
{
    gear,
    rpm,
    speedKph
};

enum class kPluginPhysicsField
{
    gearCount,
    rpmLimit,
    rpmDownshift0,
    rpmDownshift1,
    rpmDownshift2,
    rpmDownshift3,
    rpmDownshift4,
    rpmDownshift5,
    rpmDownshift6,
    rpmDownshift7,
    rpmDownshift8,
    rpmDownshift9,
    rpmDownshift10,
    rpmUpshift0,
    rpmUpshift1,
    rpmUpshift2,
    rpmUpshift3,
    rpmUpshift4,
    rpmUpshift5,
    rpmUpshift6,
    rpmUpshift7,
    rpmUpshift8,
    rpmUpshift9,
    rpmUpshift10
};

struct PluginTelemetryData
{
    std::map<kPluginTelemetryField, float> fields;
};

struct PluginPhysicsData
{
    std::map<kPluginPhysicsField, float> fields;
};
