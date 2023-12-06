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

#include <WinSock2.h>
#include <string>
#include <vector>

class WSASession
{
public:
    WSASession();
    ~WSASession();

private:
    WSAData m_data;
};

class UDPSocket
{
public:
    UDPSocket();
    ~UDPSocket();

    void sendTo(const std::string &address, unsigned short port, const char *buffer, int len, int flags = 0);
    void sendTo(sockaddr_in &address, const char *buffer, int len, int flags = 0);
    bool hasData();
    void recvData(std::vector<char> &outData, sockaddr_in &outFromAddr);
    void bindTo(unsigned short port);
    bool isBound()
    {
        return m_port != 0;
    }

    unsigned short getPort()
    {
        return m_port;
    }

    void sleep(float sleepTime);

private:
    SOCKET m_socket;
    unsigned short m_port = 0;
};
