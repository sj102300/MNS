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

	double dLatRad = toRad(impact_point.latitude - missile_->MissileLoc.latitude);
	double dLonRad = toRad(impact_point.longitude - missile_->MissileLoc.longitude);

	double dx = EARTH_RADIUS_KM * dLatRad;
	double dy = EARTH_RADIUS_KM * cos(toRad(missile_->MissileLoc.latitude)) * dLonRad;

	double len = std::sqrt(dx * dx + dy * dy);
	if (len != 0) {
		dir_lat_ = dx / len;
		dir_long_ = dy / len;
	}

	// 거리 계산 (정밀)
	double distance_km = haversine(missile_->MissileLoc.latitude, missile_->MissileLoc.longitude,
		impact_point.latitude, impact_point.longitude);

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

void MissileController::updatePosition(float speed_kmps) {  // Proportional Navigation
	if (!missile_ || !hasTarget_) return;
   	if (!(missile_->MissileState == 1 || missile_->MissileState == 5)) return;
	if (!launch_time_recorded_ || estimatedTimeToImpact_ < 0.0) return;
	if (missile_->MissileState == 2 || missile_->MissileState == 3 || missile_->MissileState == 4) {
		running_ = false;
		return;
	}

	constexpr double PN_GAIN = 3.0;
	constexpr double TIME_STEP = 0.1;

	// 종말 유도 로직 시작
	Location& msLoc = missile_->MissileLoc;
	bool isTerminalGuidance = false;

	if (!targetAircraftId_.empty() && aircraftMap_ != nullptr) {
		auto it = aircraftMap_->find(targetAircraftId_);
		if (it != aircraftMap_->end() && it->second) {
			const Location& acLoc = it->second->getLocation();

			// 거리 계산 (haversine)
			double distance_km = haversine(msLoc.latitude, msLoc.longitude, acLoc.latitude, acLoc.longitude);

			if (distance_km <= 5.0) {  // 종말 모드 시작 범위
				// 위경도 차이 → 거리(km) 방향 벡터
				double dLatRad = toRad(acLoc.latitude - msLoc.latitude);
				double dLonRad = toRad(acLoc.longitude - msLoc.longitude);

				double dx = EARTH_RADIUS_KM * dLatRad;
				double dy = EARTH_RADIUS_KM * cos(toRad(msLoc.latitude)) * dLonRad;

				double range = sqrt(dx * dx + dy * dy);
				if (range < 1e-6 || std::isnan(range)) {
					std::cerr << u8"[ERROR] 종말 유도 생략: range == 0 또는 NaN → 미사일 ID: "
							  << missile_->MissileId << "\n";
				}
				else {
					// 단위 LOS 벡터
					double los_x = dx / range;
					double los_y = dy / range;

					// 상대 속도 (V_m - V_t), 같은 방향이므로 단순 계산
					double Vm = speed_kmps;
					double Vt = speed_kmps / 2;
					double Vrel_x = Vm * dir_lat_ - Vt * los_x;
					double Vrel_y = Vm * dir_long_ - Vt * los_y;

					// LOS 각속도 λ_dot = (R × V_rel) / |R|^2
					double los_rate = (dx * Vrel_y - dy * Vrel_x) / (range * range);  // rad/s

					// 폐쇄 속도 Vc = -Vrel · LOS
					double Vc = -(Vrel_x * los_x + Vrel_y * los_y);  // > 0

					// 조향 각도 변화량 θ_dot = N * λ_dot
					double heading_change = PN_GAIN * los_rate;  // rad/s

					// dir 벡터 회전
					double angle = -heading_change * TIME_STEP;
					double cosA = cos(angle);
					double sinA = sin(angle);

					double new_dir_lat = dir_lat_ * cosA - dir_long_ * sinA;
					double new_dir_long = dir_lat_ * sinA + dir_long_ * cosA;

					dir_lat_ = new_dir_lat;
					dir_long_ = new_dir_long;
				}

				if (!hasEnteredTerminalGuidance_) {
					std::cout << u8"[종말 유도 진입] 미사일: " << missile_->MissileId
						<< u8" → 항공기: " << targetAircraftId_
						<< u8", 거리: " << distance_km << u8" km\n";
					hasEnteredTerminalGuidance_ = true;
					missile_->MissileState = 5;
				}	
			}	
		}
		else {
			std::cout << u8"[경고] 유효한 항공기 정보를 찾을 수 없음: " << targetAircraftId_ << "\n";
		}
	}
	// 종말 유도 끝

	// 위치 업데이트 (반지름 기반 → 위도/경도 단위로 변환)
	double move_km = speed_kmps * 0.1; // 0.1초당 이동 거리
	double delta_lat = (dir_lat_ * move_km) / EARTH_RADIUS_KM * 180.0 / M_PI;
	double cosLat = cos(toRad(msLoc.latitude));
	if (std::abs(cosLat) < 1e-6) cosLat = 1e-6;  // divide by 0 에러 방지
	double delta_long = (dir_long_ * move_km) / (EARTH_RADIUS_KM * cosLat) * 180.0 / M_PI;

	msLoc.latitude += delta_lat;
	msLoc.longitude += delta_long;

	auto now = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(now - launch_time_).count();

	if (elapsed > estimatedTimeToImpact_ + 10.0) {
		std::cout << u8"Estimated time to impact: " << estimatedTimeToImpact_ << "s\n" << std::endl;
		std::cout << u8"Elapsed time since launch: " << elapsed << "s\n" << std::endl;
		std::cout << u8"[Missile] 예상 충돌 시간 초과 - 자폭!\n";
		missile_->MissileState = 4; // 4번이 자폭 상태라고 가정
		running_ = false;
		return;
	}
}

// [추가]
void MissileController::setTargetAircraftId(const std::string& id) {
	targetAircraftId_ = id;
}

void MissileController::setAircraftMap(const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* aircraftMap) {
	aircraftMap_ = aircraftMap;
}

double MissileController::toRad(double deg) {
	return deg * M_PI / 180.0;
}

// 위경도 기준 거리 (km)
double MissileController::haversine(double lat1, double lon1, double lat2, double lon2) {
	double dLat = toRad(lat2 - lat1);
	double dLon = toRad(lon2 - lon1);
	lat1 = toRad(lat1);
	lat2 = toRad(lat2);

	double a = sin(dLat / 2) * sin(dLat / 2) +
		cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	return EARTH_RADIUS_KM * c;
}
