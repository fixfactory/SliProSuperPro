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
