#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#include "EngagementManager.h"

namespace TCC {
	class UdpReceiver {
	public:
		enum CommandCode {
			ModeChangeRequest = 200,
			ManualFireRequest = 201,
			EmergencyDestroyRequest = 202,
			WDLRequest = 301,
		};

		typedef struct _header {
			unsigned int commandCode_;
			int bodyLength_;
		}Header;

		typedef struct _mode_change_message {
			unsigned int mode_;
		} ModeChangeMSG;

		typedef struct _ack_header {
			unsigned int commandCode_;
			int bodyLength_;
		} AckHeader;

		typedef struct _mode_change_ack_message {
			unsigned int mode_;
		} ModeChangeAck;

		typedef struct _manual_fire_message {
			char commandId_[20];
			char targetAircraftId_[8];
		} ManualFireMSG;

		typedef struct _emergency_destroy_message {
			char commandId_[20];
			char targetMissileId_[8];
		} EmergencyDestroyMSG;

		typedef struct _wdl_message {
			char commandId_[20];
			char aircraftId_[8];
			char missileId_[8];
		} WDLMSG;

		UdpReceiver(std::string ip, int port);
		bool init(EngagementManager * engagementManager);
		void start();
		void stop();
		~UdpReceiver();

	private:
		void receive();
		void parseHeader(const char * buffer, Header& header);
		bool parseModeChangeMSG(const char* buffer, ModeChangeMSG& msg);
		bool parseManualFireMSG(const char* buffer, ManualFireMSG& msg);
		bool parseEmergencyDestroyMSG(const char* buffer, EmergencyDestroyMSG& msg);
		bool parseWdlMSG(const char* buffer, WDLMSG& msg);

		void responseChangeModeAck(unsigned int changedMode);
		void responseManualFireAck(ManualFireMSG& body);
		void responseEmergencyDestroyAck(EmergencyDestroyMSG& body);
		void responseWdlAck(WDLMSG& msg);

		std::string ip_;
		int port_;
		SOCKET serverSocket_;
		sockaddr_in recvAddr_;
		std::thread recvThread_;
		char buffer[100];
		sockaddr_in senderAddr_;
		std::atomic<bool> isRunning_;

		EngagementManager* engagementManager_;
	};
}
