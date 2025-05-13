#pragma once
#include "CoordinateGeneration.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>

class AircraftCoordinate : public CoordinateGeneration {
private:
	SOCKET udpSocket;
	sockaddr_in multicastAddr;

	double haversine(double lat1, double lon1, double lat2, double lon2);
	double toRadians(double degree);
	void initializeMultiSocket();
public:
	AircraftCoordinate();
	void makePoint(pair<double, double> startPoint, pair<double, double> finishPoint) override;
	void sendAircraftInfo(pair<double, double> currentPoint, string id, char IFF) override;
	//void scenarioReciever() override;
};