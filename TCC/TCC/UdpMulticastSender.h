
#pragma once 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "share.h"

namespace TCC {
    class UdpMulticastSender {
    public:
        UdpMulticastSender(std::string ip, int port);
        bool init();
        bool sendLaunchCommand(std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);

        typedef struct _header {
            unsigned int eventCode_;
            int bodyLength_;
        } Header;

        typedef struct _launch_command_body {
            char commandId_[20];
            char aircraftId_[8];
            char missileId_[8];
            TCC::Position impactPoint_;
        } LaunchCommandBody;

    private:
        const int serializeHeader(char* buffer, unsigned int eventCode, int bodyLength);
        const int serializeLauncCommandBody(char* buffer, std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);
        int sendByteData(const char* data, int len);

        SOCKET sock_;
        sockaddr_in mcastAddr_;
        std::string multicastIP_;
        int port_;
        sockaddr_in destAddr_;
    };
}