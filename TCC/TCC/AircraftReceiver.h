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

	typedef struct _new_aircraft_data {
		std::string aircraftId_;
		TCC::Position location_;
		bool isEnemy_;
	} NewAircraft;

	//데이터 정보를 받아서 newAircraftQueue에 push해주어야함.
	virtual void recvAircraftData() = 0;
	virtual bool popRecvQueue(NewAircraft& newAircraft) = 0;
};

class AircraftReceiver : public TCC::UdpMulticastReceiver, public IAircraftReceiver {
public:
	AircraftReceiver(const std::string& multicastIp, int port);
	void recvAircraftData() override;
	~AircraftReceiver();
	bool popRecvQueue(NewAircraft& newAircraft) override;

private:
	bool parseMsg(AircraftMSG& msg, const char* buffer, const int length);
	void receive() override;
	void pushRecvQueue(NewAircraft& newAircraft);
	std::thread recvThread_;
	std::mutex mtx_;
	std::queue<NewAircraft> recvQueue_;
};