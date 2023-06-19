#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <system_error>
#include <string>
#include <iostream>

#include "Network.h"
#include "Log.h"

WSASession::WSASession()
{
    int ret = WSAStartup(MAKEWORD(2, 2), &m_data);
    if (ret != 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "WSAStartup Failed");
    }
}

WSASession::~WSASession()
{
    WSACleanup();
}

UDPSocket::UDPSocket()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "socket() failed");
    }
}

UDPSocket::~UDPSocket()
{
    closesocket(m_socket);
}

void UDPSocket::sendTo(const std::string &address, unsigned short port, const char *buffer, int len, int flags)
{
    sockaddr_in add;
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = inet_addr(address.c_str());
    add.sin_port = htons(port);

    int ret = sendto(m_socket, buffer, len, flags, reinterpret_cast<SOCKADDR *>(&add), sizeof(add));
    if (ret < 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "sendto() failed");
    }
}

void UDPSocket::sendTo(sockaddr_in &address, const char *buffer, int len, int flags)
{
    int ret = sendto(m_socket, buffer, len, flags, reinterpret_cast<SOCKADDR *>(&address), sizeof(address));
    if (ret < 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "sendto() failed");
    }
}

bool UDPSocket::hasData()
{
    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(m_socket, &sockets);

    struct timeval timeout
    {
        .tv_sec = 0, .tv_usec = 0
    };

    int ret = select(0, &sockets, NULL, NULL, &timeout);
    if (ret < 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "select() failed");
    }
    return ret > 0;
}

void UDPSocket::recvData(std::vector<char> &outData, sockaddr_in &outFromAddr)
{
    int fromLen = sizeof(outFromAddr);
    int flags = 0;

    int ret = recvfrom(m_socket, outData.data(), (int)outData.size(), flags, reinterpret_cast<SOCKADDR *>(&outFromAddr),
                       &fromLen);
    if (ret < 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "recvfrom() failed");
    }
    outData.resize(ret);
}

void UDPSocket::bindTo(unsigned short port)
{
    sockaddr_in add;
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = htonl(INADDR_ANY);
    add.sin_port = htons(port);

    int ret = bind(m_socket, reinterpret_cast<SOCKADDR *>(&add), sizeof(add));
    if (ret < 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "bind() failed");
    }
    m_port = port;
}

void UDPSocket::sleep(float sleepTime)
{
    if (sleepTime < 0.f)
    {
        sleepTime = 0.f;
    }

    if (m_socket == INVALID_SOCKET)
    {
        // std::this_thread::sleep_for(fsec{ seconds });
        DWORD msec = (DWORD)(sleepTime * 1000);
        SleepEx(msec, 0);
        return;
    }

    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(m_socket, &sockets);

    struct timeval timeout
    {
        .tv_sec = (long)sleepTime, .tv_usec = ((long)(sleepTime * 1000) % 1000) * 1000
    };

    int ret = select(0, &sockets, NULL, NULL, &timeout);
    if (ret < 0)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "select() failed");
    }
}
