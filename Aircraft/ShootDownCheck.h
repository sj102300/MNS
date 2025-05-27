#pragma once
#include "ShootDownChecker.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>

class ShootDownCheck : public ShootDownChecker {
private:
	SOCKET udpSocket2;
	sockaddr_in multicastAddr2;

	double haversineDistance(double lat1, double lon1, double lat2, double lon2);
	double toRadians(double degree);
public:
	bool ShootDown(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) override;
	void sendSuccessInfo(std::string id, std::string missileID) override;
	std::pair<double, double> MissileReciever() override;
	std::string MissileReciever(std::string id) override;
	void initializeMultiSocket();
};