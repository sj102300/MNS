#pragma once

#include "Missile.h"

class MissileController {
public:
	MissileController();

	void setMissile(std::shared_ptr<Missile> m);

	void setTarget(Location pos);

	void start(float speed);

	void stop();

	void updateLoop(float speed);

	void updatePosition(float speed = 5.0f);

private:
	Location impact_point;
	bool hasTarget_;
	std::shared_ptr<Missile> missile_;
	bool running_ = false;
	std::thread updateThread_;
};