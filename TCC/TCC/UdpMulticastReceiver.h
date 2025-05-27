
#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>  // �� �߰� �ʿ�
#include <iostream>
#include <thread>
#include "share.h"
#include "AircraftManager.h"

class MissileManager;

namespace TCC {
	class UdpMulticastReceiver {
	public:
		UdpMulticastReceiver(const std::string& multicastIp, int port);

		typedef struct _header {
			unsigned int eventCode_;
			int bodyLength_;
		} Header;

		typedef struct _aircraft_message {
			char aircraftId_[8];
			Position location_;
			char ourOrEnemy_;
		}AircraftMSG;

		typedef struct _missile_message {
			unsigned int eventCode_;
			unsigned int bodyLength_;
			char missileId[8];
			unsigned int status_;
			Position location_;
		} MissileMSG;

		bool init(AircraftManager* aircraftManager, MissileManager* missileManager);
		void start();

	private:
		AircraftManager* aircraftManager_;		//�����Ϳ� �ΰ� �־��ְ� üũ����ߵ�.
		MissileManager* missileManager_;		//�����Ϳ� �ΰ� �־��ְ� üũ����ߵ�.
		//ISuccessReceiver* successRecv_;
		//IMissileReceiver* missileRecv_;
		std::thread recvThread_;
		std::string multicastIp_;
		int port_;
		SOCKET serverSocket_;
		char buffer[100];
		
		void receive();
		void parseHeader(Header& header);
		bool parseReceivedAircraftMSG(const char* buffer, AircraftManager::NewAircraft& newAircraft, int length);
		bool parseReceivedMissileMSG(const char* buffer, MissileMSG& data, int length);
	};
}
