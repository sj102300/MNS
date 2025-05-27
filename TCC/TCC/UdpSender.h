#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include "AircraftManager.h"

#pragma comment(lib, "ws2_32.lib")

namespace TCC{
    class UdpSender{
    public:
        UdpSender(const std::string& ip, int port);
        ~UdpSender();

        bool init();
        int sendByteData(const char * buffer, int length);
        const int serializeHeader(char* buffer, const int commandCode, int bodyLength);
        
        bool sendAircraftData(AircraftManager::NewAircraftWithIP& data);
        const int serializeAircraftSender(char* buffer, AircraftManager::NewAircraftWithIP& data);

    private:
        std::string ip_;
        int port_;
        SOCKET sock_;
        sockaddr_in targetAddr_;
    };
}