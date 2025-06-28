#pragma once
#define _USE_MATH_DEFINES
#include "MissileController.h"
#include <cmath>
#include <memory>
#include <thread>
#include <iostream>
#include <utility>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void MissileController::setMovementState(MovementState state) {
	currentMovementState_ = state;
	if (state == MovementState::Straight) {
		std::cout << u8"[MissileController] 현재 상태: 직진 중\n";
	}
	else if (state == MovementState::Turning) {
		std::cout << u8"[MissileController] 현재 상태: 회전 중\n";
	}
}
MissileController::MovementState MissileController::getMovementState() const {
	return currentMovementState_;
}
double MissileController::compute_heading(double from_lat, double from_lon, double to_lat, double to_lon) {
	constexpr double PI = 3.14159265358979323846;
	double dLon = (to_lon - from_lon) * PI / 180.0;
	double lat1 = from_lat * PI / 180.0;
	double lat2 = to_lat * PI / 180.0;
	double y = sin(dLon) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
	return fmod(atan2(y, x) + 2 * PI, 2 * PI);
}
double MissileController::Cal_RateOfTurn(double current_w, double heading_error, double dt) {
	constexpr double PI = 3.14159265358979323846;
	double aoa = 0.05236; // 3도(rad)
	double v = 2000.0;
	double air_density = 0.4135;

	double L = air_density * v * v * aoa / 2.0;
	double l = (4.61 * 0.5) / (4.61 * 0.25);
	double T = L * l;
	double I = 400 * (3 * 0.275 * 0.275 + 4.61 * 4.61) / 12;

	double max_angular_accel = T / I;

	double Kp = 1.0;
	double Kd = 2.0;

	double angular_accel = Kp * heading_error - Kd * current_w;

	// 실제 물리 제약 반영
	angular_accel = std::clamp(angular_accel, -max_angular_accel, max_angular_accel);

	double new_w = current_w + angular_accel * dt;
	return new_w;
}
void MissileController::updateTurnToNewTarget(float speed_kmps) {
	if (!missile_ || !hasTarget_) return;

	constexpr double PI = 3.14159265358979323846;
	constexpr double dt = 0.1;
	constexpr double EARTH_RADIUS_M = 6371000.0;

	double& lat = missile_->MissileLoc.latitude;
	double& lon = missile_->MissileLoc.longitude;

	// 1. 초기 heading 계산
	if (!turning_initialized_) {
		turning_heading_ = compute_heading(lat, lon, impact_point.latitude, impact_point.longitude);
		turning_initialized_ = true;
	}

	// 2. 목표 방향 계산
	double desired_heading = compute_heading(lat, lon, impact_point.latitude, impact_point.longitude);
	double heading_error = desired_heading - turning_heading_;

	if (heading_error > PI) heading_error -= 2 * PI;
	if (heading_error < -PI) heading_error += 2 * PI;

	// 3. 선회율 계산
	turning_w_ = Cal_RateOfTurn(turning_w_, heading_error, dt);

	// 4. 현재 heading 업데이트
	turning_heading_ += turning_w_ * dt;
	turning_heading_ = fmod(turning_heading_ + 2 * PI, 2 * PI);

	// 5. 현재 위치 업데이트
	auto new_pos = update_latlon(turning_heading_, dt, lat, lon);
	lat = new_pos.first;
	lon = new_pos.second;

	// 6. 도달 체크 → 직선 상태로 전환
	double distance_km = haversine(lat, lon, impact_point.latitude, impact_point.longitude);
	if (distance_km < 0.05 || distance_km <= 5.0) {  // 50m 이내면 전환
		std::cout << u8"[INFO] 목표 근처 도달, 직선 운동 상태로 전환(종말 유도)\n";
		setMovementState(MovementState::Straight);
		turning_initialized_ = false;  // 초기화

		if (distance_km <= 5.0) {
			updatePosition(speed_kmps);  // 종말 유도 상태로 전환
		}
	}
}
std::pair<double, double> MissileController::update_latlon(double heading, double dt, double lat_deg, double lon_deg) {
	double v = 2000.0;
	constexpr double EARTH_RADIUS_M = 6371000.0;
	double lat = lat_deg * M_PI / 180.0;
	double lon = lon_deg * M_PI / 180.0;

	double dNorth = v * cos(heading) * dt;
	double dEast = v * sin(heading) * dt;

	double dLat = dNorth / EARTH_RADIUS_M;
	double dLon = dEast / (EARTH_RADIUS_M * cos(lat));

	double new_lat = lat + dLat;
	double new_lon = lon + dLon;

	return { new_lat * 180.0 / M_PI, new_lon * 180.0 / M_PI };
}
void MissileController::forceTurnToNewTarget() {
	isTurningToNewTarget_ = true;
	launch_time_recorded_ = true;
	hasTarget_ = true;
}


MissileController::MissileController(DestroyedAircraftsTracker* tracker)
	:impact_point({ 0, 0, 10 }), hasTarget_(false) {
	tracker_ = tracker;
}

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
	//speed_kmps_init_ = speed;
	running_ = true;
	updateThread_ = std::thread(&MissileController::updateLoop, this, speed);
}

void MissileController::updateLoop(float speed) {
	while (running_) {
		if (currentMovementState_ == MovementState::Straight) updatePosition(speed); // 위치 갱신
		else if (currentMovementState_ == MovementState::Turning) updateTurnToNewTarget(speed);//새로운 곡선 함수 실행 해야 함
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
	if (!(missile_->MissileState == 1 || missile_->MissileState == 5 || missile_->MissileState == 7)) return;
	if (!launch_time_recorded_ || estimatedTimeToImpact_ < 0.0) return;
	if (missile_->MissileState == 2 || missile_->MissileState == 3 || missile_->MissileState == 4) {
		running_ = false;
		return;
	}
	//if (missile_->MissileState == 7) {
	//	state7Count_++;
	//	if (state7Count_ >= 3) {
	//		missile_->MissileState = 1;
	//		state7Count_ = 0;  // 초기화
	//	}
	//	else {
	//		return;  // 3회 호출 전까진 나머지 로직 실행 안 함
	//	}
	//}
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

			if (tracker_ != nullptr && !tracker_->isDestroyedAircraft(targetAircraftId_)) {


				if (distance_km <= 5.0) {  // 종말 모드 시작 범위
					// 위경도 차이 → 거리(km) 방향  벡터

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
						/*std::cout << u8"[종말 유도 진입] 미사일: " << missile_->MissileId
							<< u8" → 항공기: " << targetAircraftId_
							<< u8", 거리: " << distance_km << u8" km\n";*/
						hasEnteredTerminalGuidance_ = true;

						missile_->MissileState = 5; // 5번이 종말 유도 상태라고 가정
					}
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