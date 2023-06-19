#pragma once

#include <vector>
#include <chrono>

#include "PhysicsNG/rbr.telemetry.data.TelemetryData.h"
#include "PluginInterface.h"

using RBRTelemetryData = rbr::telemetry::data::TelemetryData;

class WSASession;
class UDPSocket;

class TelemetryManager
{
public:
    static TelemetryManager &getSingleton();

    TelemetryManager();
    ~TelemetryManager();

    void init();
    void deinit();

    bool fetchTelemetryData();

    bool isReceivingTelemetry() const;
    const plugin::TelemetryData &getTelemetryData() const;
    const RBRTelemetryData &getRBRTelemetryData() const;

private:
    void recvTelemetry();

    WSASession *m_session = nullptr;
    UDPSocket *m_udpSocket = nullptr;
    std::vector<char> m_recvBuf;
    bool m_receivingTelemetry = false;
    plugin::TelemetryData m_telemetryData{};
    RBRTelemetryData m_rbrTelemetryData{};

    using time_point = std::chrono::steady_clock::time_point;
    time_point m_lastDataTime = {};
};
