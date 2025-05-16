#pragma once

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "CoordinateGeneration.h"

#undef byte

class AircraftCoordinate : public CoordinateGeneration {
private:
	SOCKET udpSocket;
	sockaddr_in multicastAddr;

	double haversine(double lat1, double lon1, double lat2, double lon2);
	double toRadians(double degree);
public:
	vector<double> makeStartOpt(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) override;
	pair<double, double> makePoint(double dx, double dy, pair<double, double> currentPoint) override;
	void sendAircraftInfo(pair<double, double> currentPoint, string id, char IFF) override;
	vector<vector<string>> scenarioReciever() override;
	void initializeMultiSocket();
};