
#pragma once
#include "MissileController.h"
#include <cmath>
#include <memory>
#include <thread>
#include <iostream>

MissileController::MissileController()
	:impact_point({0, 0, 10}),hasTarget_(false){}

void MissileController::setMissile(std::shared_ptr<Missile> m) {
	missile_ = m;
}

void MissileController::setTarget(Location pos) {
	impact_point = pos;
	hasTarget_ = true;
}

void MissileController::start(float speed) {
	running_ = true;
	updateThread_ = std::thread(&MissileController::updateLoop, this, speed);
}
void MissileController::updateLoop(float speed) {
	while (running_) {
		updatePosition(speed); // 위치 갱신
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 1초 주기
	}
}

void MissileController::stop() {
	hasTarget_ = false;
	running_ = false;
	if (updateThread_.joinable())
		updateThread_.join();
}

void MissileController::updatePosition(float speed ) {
	if (!hasTarget_ || missile_->MissileState != 1) return;
	double dx = impact_point.altitude - missile_->MissileLoc.altitude;
	double dy = impact_point.latitude - missile_->MissileLoc.latitude;
	double dz = impact_point.longitude - missile_->MissileLoc.longitude;

	double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
	
	double move = speed;
	missile_->MissileLoc.altitude += (dx / dist) * move;
	missile_->MissileLoc.latitude += (dy/ dist) * move;
	missile_->MissileLoc.longitude += (dz / dist) * move;
}