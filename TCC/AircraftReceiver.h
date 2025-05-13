#pragma once

#include "UdpMulticastReceiver.h"
#include "share.h"
#include <regex>
#include <queue>
#include <mutex>

class IAircraftReceiver {
public:
	typedef struct _aircraft_message {
		char aircraftId[8]; //8ÀÚ
		TCC::Position location;
		bool isEnemy;
	} AircraftMSG;

	virtual void getAircraftData() = 0;
	virtual void pushRecvQueue(IAircraftReceiver::AircraftMSG& msg) = 0;
	virtual bool popRecvQueue(IAircraftReceiver::AircraftMSG& msg) = 0;
};

class AircraftReceiver : public TCC::UdpMulticastReceiver, public IAircraftReceiver {
public:
	AircraftReceiver(const std::string& multicastIp, int port);
	void getAircraftData() override;
	void pushRecvQueue(IAircraftReceiver::AircraftMSG &msg) override;
	bool popRecvQueue(IAircraftReceiver::AircraftMSG &msg) override;

private:
	bool parseMsg(AircraftMSG& msg, const char* buffer, const int length);
	void receive() override;
	std::mutex mtx_;
	std::queue<IAircraftReceiver::AircraftMSG> recvQueue_;
};