#pragma once

#include <vector>
#include <chrono>

#include "Timing.h"

#include "PhysicsNG/rbr.telemetry.data.TelemetryData.h"

using TelemetryData = rbr::telemetry::data::TelemetryData;

class WSASession;
class UDPSocket;

class TelemetryManager
{
public:
    TelemetryManager();
    ~TelemetryManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTime);
    void sleep(float sleepTime);

    bool isReceivingTelemetry() const { return m_receivingTelemetry; }
    const TelemetryData* getTelemetryData() const { return &m_telemetryData; }

private:
    void recvTelemetry();

    WSASession* m_session = nullptr;
    UDPSocket* m_udpSocket = nullptr;
    std::vector<char> m_recvBuf;
    bool m_receivingTelemetry = false;
    TelemetryData m_telemetryData = {};
    
    using time_point = std::chrono::steady_clock::time_point;
    time_point m_lastDataTime = {};
};