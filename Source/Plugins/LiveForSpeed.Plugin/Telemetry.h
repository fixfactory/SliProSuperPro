//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2024 Fixfactory
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

#include "PluginInterface.h"

#include "json/json.hpp"
using json = nlohmann::json;

class WSASession;
class UDPSocket;
struct OutGaugePack;

class TelemetryManager
{
public:
    static TelemetryManager &getSingleton();

    TelemetryManager();
    ~TelemetryManager();

    void init();
    void deinit();

    bool fetchTelemetryData();
    const plugin::TelemetryData &getTelemetryData() const;
    const plugin::PhysicsData &getPhysicsData() const;

private:
    WSASession *m_session = nullptr;
    UDPSocket *m_udpSocket = nullptr;
    std::vector<char> m_recvBuf;
    bool m_receivingTelemetry{ false };

    using time_point = std::chrono::steady_clock::time_point;
    time_point m_lastDataTime = {};

    plugin::TelemetryData m_telemetryData{};
    plugin::PhysicsData m_physicsData{};

    json m_carData;
    std::string m_carId;
    std::string m_lastCarId;
    std::string m_carName;

    bool m_useInSim{ false };
    std::string m_inSimHostname;
    int m_inSimPort{ 0 };
    std::string m_inSimPassword;

    int m_outGaugePort{ 0 };

    void readConfig();
    void readCarData();
    void parseCarData();

    void openInSim();
    void closeInSim();

    void initOutGauge();
    void deinitOutGauge();
    bool recvOutGauge(OutGaugePack &outGaugePack);
};
