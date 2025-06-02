
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
		~UdpMulticastReceiver();
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

		typedef struct _engagement_success_message {
			char engagementSuccessId_[20];
			char targetAircraftId_[8];
			char targetMissileId_[8];
		} EngagementSuccessMSG;

		bool init(AircraftManager* aircraftManager, MissileManager* missileManager, EngagementManager* engagementManager);
		void start();
		void stop();

	private:

		enum EventCode {
			//AircraftData = 1001,		ATS���� ������ �װ���
			FindTargetEvent = 1002,
			EngagementSuccess = 2003,
			MissileStatus = 3001
		};

		AircraftManager* aircraftManager_;		//�����Ϳ� �ΰ� �־��ְ� üũ����ߵ�.
		MissileManager* missileManager_;		//�����Ϳ� �ΰ� �־��ְ� üũ����ߵ�.
		EngagementManager* engagementManager_;	//�����Ϳ� �ΰ� �־��ְ� üũ����ߵ�.
		//ISuccessReceiver* successRecv_;
		//IMissileReceiver* missileRecv_;
		std::thread recvThread_;
		std::string multicastIp_;
		int port_;
		SOCKET serverSocket_;
		char buffer[100];
		std::atomic<bool> isRunning_;
		sockaddr_in senderAddr_;

		void receive();
		void parseHeader(Header& header);
		bool parseReceivedAircraftMSG(const char* buffer, AircraftManager::NewAircraft& newAircraft, int length);
		bool parseReceivedMissileMSG(const char* buffer, MissileMSG& data, int length);
		bool parseReceivedEngagementSuccessMSG(const char* buffer, EngagementSuccessMSG& msg, int length);
		bool responseEngagementSuccessAck(EngagementSuccessMSG& msg);

	};
}
