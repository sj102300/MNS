
#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>  // 꼭 추가 필요
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

#pragma pack(push,1)
		typedef struct _header {
			unsigned int eventCode_;
			int bodyLength_;
		} Header;
#pragma pack(pop)

#pragma pack(push,1)
		typedef struct _aircraft_message {
			char aircraftId_[8];
			Position location_;
			char ourOrEnemy_;
		}AircraftMSG;
#pragma pack(pop)

#pragma pack(push,1)
		typedef struct _missile_message {
			char missileId[8];
			unsigned int status_;
			Position location_;
		} MissileMSG;
#pragma pack(pop)

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
			//AircraftData = 1001,		ATS에서 들어오는 항공기
			FindTargetEvent = 1002,
			EngagementSuccess = 2003,
			MissileStatus = 3001
		};

		AircraftManager* aircraftManager_;		//포인터에 널값 넣어주고 체크해줘야됨.
		MissileManager* missileManager_;		//포인터에 널값 넣어주고 체크해줘야됨.
		EngagementManager* engagementManager_;	//포인터에 널값 넣어주고 체크해줘야됨.
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
