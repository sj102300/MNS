
#pragma once 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include "share.h"


namespace TCC {
    class UdpMulticastSender {
    public:
        UdpMulticastSender(std::string ip, int port);
        bool init();
        void sendLaunchCommand(std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);
        void sendEmergencyDestroyCommand(std::string& commandId, std::string& missileId);

        enum EventCode{
            launchCommand = 2001,
            emergencyDestroyCommand = 2004,
        };

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

        typedef struct _emergency_destroy_command_body {
            char commandId_[20];
            char missileId_[8];
        } EmergencyDestroyCommandBody;

    private:
        const int serializeHeader(char* buffer, unsigned int eventCode, int bodyLength);
        const int serializeLauncCommandBody(char* buffer, std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);
        const int serializeEmergencyDestroyCommandBody(char* buffer, std::string& commandId, std::string& missileId);
        const int sendByteData(const char* data, int len);
		void sendUntilReceiveAck(const char* buffer, int length);

        SOCKET sock_;
        sockaddr_in mcastAddr_;
        std::string multicastIP_;
        int port_;
        sockaddr_in destAddr_;
    };
}