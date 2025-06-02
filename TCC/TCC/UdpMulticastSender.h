
#pragma once 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include "share.h"
#include <mutex>
#include <unordered_map>


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

        // Ack 결과를 외부에서 확인할 수 있도록 getter 제공 (예시)
        bool waitForAckResult(const std::string& missileId, int timeoutMs = 3000);
        void setAckResult(const std::string& missileId, bool result);

    private:
        const int serializeHeader(char* buffer, unsigned int eventCode, int bodyLength);
        const int serializeLauncCommandBody(char* buffer, std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint);
        const int serializeEmergencyDestroyCommandBody(char* buffer, std::string& commandId, std::string& missileId);
        const int sendByteData(const char* data, int len);
		bool sendUntilReceiveAck(const char* buffer, int length);

        std::mutex ackMtx_;
        std::condition_variable ackCv_;
        std::unordered_map<std::string, bool> ackResults_;  // missileId별 Ack 결과 저장

        SOCKET sock_;
        sockaddr_in mcastAddr_;
        std::string multicastIP_;
        int port_;
        sockaddr_in destAddr_;
    };
}