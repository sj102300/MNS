#pragma once

#include "UdpMulticastReceiver.h"
#include "share.h"

class AircraftReceiver : public TCC::UdpMulticastReceiver {
private:
	typedef struct _aircraft_message {
		std::string aircraftId; //8ÀÚ
		TCC::Position location;
		bool isEnemy;
	}AircraftMSG;

public:
	AircraftReceiver(std::string multicastIp, int port);

	void parseMsg(AircraftMSG& msg, const char* buffer, int length);

	void receiveLoop() override;

	void start() override;

};