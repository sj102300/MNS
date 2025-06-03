#pragma once

#include "Missile.h"
#include <chrono>

class MissileController {
public:
	MissileController();

	void setMissile(std::shared_ptr<Missile> m);

	void setTarget(Location pos);

	void start(float speed);

	void stop();

	void updateLoop(float speed);

	void updatePosition(float speed = 5.0f);

	// gtest를 위한 코드 2줄
public:
	bool hasTarget() const { return hasTarget_; }
	double getEstimatedTimeToImpact() const { return estimatedTimeToImpact_; }

private:
	Location impact_point;
	bool hasTarget_;
	std::shared_ptr<Missile> missile_;
	bool running_ = false;
	std::thread updateThread_;
	double dir_lat_ = 0.0;   // 방향 단위 벡터 (위도 방향)
	double dir_long_ = 0.0;  // 방향 단위 벡터 (경도 방향)
	std::chrono::steady_clock::time_point launch_time_;
	bool launch_time_recorded_ = false;
	double estimatedTimeToImpact_ = -1.0;
};