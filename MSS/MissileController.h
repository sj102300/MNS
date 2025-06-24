#pragma once

#include "Missile.h"
#include "Aircraft.h"  // [�߰�] Aircraft
#include <chrono>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <cmath>
#include <unordered_set>
#include "DestroyedAircrafts.h"
#define EARTH_RADIUS_KM 6371.0

class Aircraft;  // [�߰�] ����

class MissileController {
public:
	MissileController(DestroyedAircraftsTracker * tracker);

	void setMissile(std::shared_ptr<Missile> m);

	void setTarget(Location pos);  // ���� ����Ʈ ����Ʈ��

	void start(float speed);

	void stop();

	void updateLoop(float speed);

	void updatePosition(float speed = 5.0f);

	// [�߰�] ������ �װ��� ID, �� ����
	void setTargetAircraftId(const std::string& aircraftId);
	void setAircraftMap(const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* aircraftMap);

	double toRad(double deg);
	double haversine(double lat1, double lon1, double lat2, double lon2);

	// gtest�� ���� �ڵ� 2��
	bool hasTarget() const { return hasTarget_; }
	double getEstimatedTimeToImpact() const { return estimatedTimeToImpact_; }

private:
	int state7Count_ = 0;
	Location impact_point;
	bool hasTarget_;
	std::shared_ptr<Missile> missile_;
	bool running_ = false;
	std::thread updateThread_;
	double dir_lat_ = 0.0;   // ���� ���� ���� (���� ����)
	double dir_long_ = 0.0;  // ���� ���� ���� (�浵 ����)
	std::chrono::steady_clock::time_point launch_time_;
	bool launch_time_recorded_ = false;
	double estimatedTimeToImpact_ = -1.0;

	// [�߰�]
	std::string targetAircraftId_;
	const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* aircraftMap_ = nullptr;
	bool hasEnteredTerminalGuidance_ = false;

	DestroyedAircraftsTracker* tracker_;

	// WDL ���� �߰�
	//bool isTurningToNewTarget_ = false;
	//double target_heading_rad_ = 0.0;

};