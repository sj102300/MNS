#pragma once
#include <string>
#include <utility>

class ShootDownChecker {
public:
	virtual bool ShootDown(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) = 0;
	virtual void sendSuccessInfo(std::string id, std::string missileID) = 0;
	virtual std::pair<double, double> MissileReciever() = 0;
	virtual std::string MissileReciever(std::string id) = 0;
};