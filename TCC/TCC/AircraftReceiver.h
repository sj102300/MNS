#pragma once

#include "UdpMulticastReceiver.h"
#include "share.h"
#include <regex>
#include <queue>
#include <mutex>

class IAircraftReceiver {
public:
	typedef struct _aircraft_message {
		char aircraftId[8]; //8자
		TCC::Position location;
		char friend_or_our;
	} AircraftMSG;

	//데이터 정보를 받아서 newAircraftQueue에 push해주어야함.
	virtual void getAircraftData() = 0;
};

class AircraftManager;

class AircraftReceiver : public TCC::UdpMulticastReceiver, public IAircraftReceiver {
public:
	AircraftReceiver(const std::string& multicastIp, int port);
	void getAircraftData() override;
	~AircraftReceiver();

private:
	bool parseMsg(AircraftMSG& msg, const char* buffer, const int length);
	void receive() override;
	std::thread recvThread_;
	AircraftManager* aircraftmanager_;
};