#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include "AircraftManager.h"
#include "MissileManager.h"
#include "UdpMulticastReceiver.h"
#include "UdpReceiver.h"

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

        bool sendMissileData(UdpMulticastReceiver::MissileMSG& data);
        const int serializeMissileSender(char* buffer, UdpMulticastReceiver::MissileMSG& data);

        /*bool sendEmergencyDestroy(std::string commnadId, std::string missileId);
        const int serializeEmergencySender(char* buffer, std::string commnadId, std::string missileId);*/

		bool sendLaunchCommand(std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);
		const int serializeLaunchCommandBody(char* buffer, std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);

    private:
        std::string ip_;
        int port_;
        SOCKET sock_;
        sockaddr_in targetAddr_;

    };
}