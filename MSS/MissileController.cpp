#pragma once
#define _USE_MATH_DEFINES
#include "MissileController.h"
#include <cmath>
#include <memory>
#include <thread>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MissileController::MissileController()
	:impact_point({0, 0, 10}),hasTarget_(false){}

void MissileController::setMissile(std::shared_ptr<Missile> m) {
	missile_ = m;
}

void MissileController::setTarget(Location pos) {
	impact_point = pos;
	hasTarget_ = true;

	double dx = impact_point.latitude - missile_->MissileLoc.latitude;
	double dy = impact_point.longitude - missile_->MissileLoc.longitude;
	double len = std::sqrt(dx * dx + dy * dy);

	if (len != 0) {
		dir_lat_ = dx / len;
		dir_long_ = dy / len;
	}
	// 거리(km) 단위로 환산
	//constexpr double lat_km_per_deg = 111.0;
	//double long_km_per_deg = 111.0 * cos(missile_->MissileLoc.latitude * M_PI / 180.0);
	double lat_rad = missile_->MissileLoc.latitude * M_PI / 180.0;

	double lat_km_per_deg = 111.13295 - 0.55982 * std::cos(2 * lat_rad) + 0.00117 * std::cos(4 * lat_rad);
	double long_km_per_deg = 111.41288 * std::cos(lat_rad) - 0.09350 * std::cos(3 * lat_rad) + 0.00012 * std::cos(5 * lat_rad);

	double dx_km = dx * lat_km_per_deg;
	double dy_km = dy * long_km_per_deg;
	double distance_km = std::sqrt(dx_km * dx_km + dy_km * dy_km);

	estimatedTimeToImpact_ = (2.0 > 0) ? (distance_km / 2.0) : -1.0;
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

void MissileController::updatePosition(float speed_kmps) {
	if (missile_->MissileState == 2 || missile_->MissileState == 3 || missile_->MissileState == 4) {
		running_ = false;
		return;
	}
	if (!hasTarget_ || missile_->MissileState != 1) return;
	if (!launch_time_recorded_ || estimatedTimeToImpact_ < 0.0) return;

	constexpr double lat_km_per_deg = 111.0;
	double lat = missile_-> MissileLoc.latitude;
	double long_km_per_deg = 111.0 * cos(lat * M_PI / 180.0);

	double delta_lat = (dir_lat_ * speed_kmps* 0.1) / lat_km_per_deg;
	double delta_long = (dir_long_ * speed_kmps* 0.1) / long_km_per_deg;

	

	missile_->MissileLoc.latitude += delta_lat;
	missile_->MissileLoc.longitude += delta_long;

	auto now = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(now - launch_time_).count();

	if (elapsed > estimatedTimeToImpact_ + 2.0) {
		std::cout << u8"Estimated time to impact: " << estimatedTimeToImpact_ << "s" << std::endl;
		std::cout << u8"Elapsed time since launch: " << elapsed << "s" << std::endl;
		std::cout << u8"[Missile] 예상 충돌 시간 초과 - 자폭!\n";
		missile_->MissileState = 4; // 4번이 자폭 상태라고 가정
		running_ = false;
		return;
	}

}

