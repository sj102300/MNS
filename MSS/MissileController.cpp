
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
	float speed = 0.1;

	double dx = impact_point.latitude - missile_->MissileLoc.latitude;
	double dy = impact_point.longitude - missile_->MissileLoc.longitude;
	double len = std::sqrt(dx * dx + dy * dy);

	if (len != 0) {
		dir_lat_ = dx / len;
		dir_long_ = dy / len;
	}
	estimatedTimeToImpact_ = (speed > 0) ? (len / speed) : -1.0;
	launch_time_ = std::chrono::steady_clock::now();
	launch_time_recorded_ = true;
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

void MissileController::updatePosition(float speed) {
	if (missile_->MissileState == 2 || missile_->MissileState == 3 || missile_->MissileState == 4) {
		running_ = false;
		return;
	}
	if (!hasTarget_ || missile_->MissileState != 1) return;
	if (!launch_time_recorded_ || estimatedTimeToImpact_ < 0.0) return;

	missile_->MissileLoc.latitude += dir_lat_ * speed;
	missile_->MissileLoc.longitude += dir_long_ * speed;


	auto now = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(now - launch_time_).count();

	std::cout << u8"Estimated time to impact: " << estimatedTimeToImpact_ << "s" << std::endl;
	std::cout << u8"Elapsed time since launch: " << elapsed << "s" << std::endl;

	// 고정된 도달 시간 기준으로 자폭 조건 체크
	if ((elapsed - estimatedTimeToImpact_) > 2.0) {
		missile_->MissileState = 4;  // 자폭
	}
}