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

    void sendTo(const std::string& address, unsigned short port, const char* buffer, int len, int flags = 0);
    void sendTo(sockaddr_in& address, const char* buffer, int len, int flags = 0);
    bool hasData();
    void recvData(std::vector<char>& outData, sockaddr_in& outFromAddr);
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
