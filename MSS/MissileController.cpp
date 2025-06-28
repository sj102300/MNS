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
		std::cout << u8"[MissileController] ���� ����: ���� ��\n";
	}
	else if (state == MovementState::Turning) {
		std::cout << u8"[MissileController] ���� ����: ȸ�� ��\n";
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
	double aoa = 0.05236; // 3��(rad)
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

	// ���� ���� ���� �ݿ�
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

	// 1. �ʱ� heading ���
	if (!turning_initialized_) {
		turning_heading_ = compute_heading(lat, lon, impact_point.latitude, impact_point.longitude);
		turning_initialized_ = true;
	}

	// 2. ��ǥ ���� ���
	double desired_heading = compute_heading(lat, lon, impact_point.latitude, impact_point.longitude);
	double heading_error = desired_heading - turning_heading_;

	if (heading_error > PI) heading_error -= 2 * PI;
	if (heading_error < -PI) heading_error += 2 * PI;

	// 3. ��ȸ�� ���
	turning_w_ = Cal_RateOfTurn(turning_w_, heading_error, dt);

	// 4. ���� heading ������Ʈ
	turning_heading_ += turning_w_ * dt;
	turning_heading_ = fmod(turning_heading_ + 2 * PI, 2 * PI);

	// 5. ���� ��ġ ������Ʈ
	auto new_pos = update_latlon(turning_heading_, dt, lat, lon);
	lat = new_pos.first;
	lon = new_pos.second;

	// 6. ���� üũ �� ���� ���·� ��ȯ
	double distance_km = haversine(lat, lon, impact_point.latitude, impact_point.longitude);
	if (distance_km < 0.05 || distance_km <= 5.0) {  // 50m �̳��� ��ȯ
		std::cout << u8"[INFO] ��ǥ ��ó ����, ���� � ���·� ��ȯ(���� ����)\n";
		setMovementState(MovementState::Straight);
		turning_initialized_ = false;  // �ʱ�ȭ

		if (distance_km <= 5.0) {
			updatePosition(speed_kmps);  // ���� ���� ���·� ��ȯ
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

	// �Ÿ� ��� (����)
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
		if (currentMovementState_ == MovementState::Straight) updatePosition(speed); // ��ġ ����
		else if (currentMovementState_ == MovementState::Turning) updateTurnToNewTarget(speed);//���ο� � �Լ� ���� �ؾ� ��
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 1�� �ֱ�
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
	//		state7Count_ = 0;  // �ʱ�ȭ
	//	}
	//	else {
	//		return;  // 3ȸ ȣ�� ������ ������ ���� ���� �� ��
	//	}
	//}
	constexpr double PN_GAIN = 3.0;
	constexpr double TIME_STEP = 0.1;

	// ���� ���� ���� ����
	Location& msLoc = missile_->MissileLoc;
	bool isTerminalGuidance = false;

	if (!targetAircraftId_.empty() && aircraftMap_ != nullptr) {

		auto it = aircraftMap_->find(targetAircraftId_);
		if (it != aircraftMap_->end() && it->second) {
			const Location& acLoc = it->second->getLocation();

			// �Ÿ� ��� (haversine)
			double distance_km = haversine(msLoc.latitude, msLoc.longitude, acLoc.latitude, acLoc.longitude);

			if (tracker_ != nullptr && !tracker_->isDestroyedAircraft(targetAircraftId_)) {


				if (distance_km <= 5.0) {  // ���� ��� ���� ����
					// ���浵 ���� �� �Ÿ�(km) ����  ����

					double dLatRad = toRad(acLoc.latitude - msLoc.latitude);
					double dLonRad = toRad(acLoc.longitude - msLoc.longitude);

					double dx = EARTH_RADIUS_KM * dLatRad;
					double dy = EARTH_RADIUS_KM * cos(toRad(msLoc.latitude)) * dLonRad;

					double range = sqrt(dx * dx + dy * dy);
					if (range < 1e-6 || std::isnan(range)) {
						std::cerr << u8"[ERROR] ���� ���� ����: range == 0 �Ǵ� NaN �� �̻��� ID: "
							<< missile_->MissileId << "\n";
					}
					else {
						// ���� LOS ����
						double los_x = dx / range;
						double los_y = dy / range;

						// ��� �ӵ� (V_m - V_t), ���� �����̹Ƿ� �ܼ� ���
						double Vm = speed_kmps;
						double Vt = speed_kmps / 2;
						double Vrel_x = Vm * dir_lat_ - Vt * los_x;
						double Vrel_y = Vm * dir_long_ - Vt * los_y;

						// LOS ���ӵ� ��_dot = (R �� V_rel) / |R|^2
						double los_rate = (dx * Vrel_y - dy * Vrel_x) / (range * range);  // rad/s

						// ��� �ӵ� Vc = -Vrel �� LOS
						double Vc = -(Vrel_x * los_x + Vrel_y * los_y);  // > 0

						// ���� ���� ��ȭ�� ��_dot = N * ��_dot
						double heading_change = PN_GAIN * los_rate;  // rad/s

						// dir ���� ȸ��
						double angle = -heading_change * TIME_STEP;
						double cosA = cos(angle);
						double sinA = sin(angle);

						double new_dir_lat = dir_lat_ * cosA - dir_long_ * sinA;
						double new_dir_long = dir_lat_ * sinA + dir_long_ * cosA;

						dir_lat_ = new_dir_lat;
						dir_long_ = new_dir_long;
					}

					if (!hasEnteredTerminalGuidance_) {
						/*std::cout << u8"[���� ���� ����] �̻���: " << missile_->MissileId
							<< u8" �� �װ���: " << targetAircraftId_
							<< u8", �Ÿ�: " << distance_km << u8" km\n";*/
						hasEnteredTerminalGuidance_ = true;

						missile_->MissileState = 5; // 5���� ���� ���� ���¶�� ����
					}
				}
			}
		}
		else {
			std::cout << u8"[���] ��ȿ�� �װ��� ������ ã�� �� ����: " << targetAircraftId_ << "\n";
		}
	}
	// ���� ���� ��


	// ��ġ ������Ʈ (������ ��� �� ����/�浵 ������ ��ȯ)
	double move_km = speed_kmps * 0.1; // 0.1�ʴ� �̵� �Ÿ�
	double delta_lat = (dir_lat_ * move_km) / EARTH_RADIUS_KM * 180.0 / M_PI;
	double cosLat = cos(toRad(msLoc.latitude));
	if (std::abs(cosLat) < 1e-6) cosLat = 1e-6;  // divide by 0 ���� ����
	double delta_long = (dir_long_ * move_km) / (EARTH_RADIUS_KM * cosLat) * 180.0 / M_PI;

	msLoc.latitude += delta_lat;
	msLoc.longitude += delta_long;

	auto now = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(now - launch_time_).count();

	if (elapsed > estimatedTimeToImpact_ + 10.0) {
		std::cout << u8"Estimated time to impact: " << estimatedTimeToImpact_ << "s\n" << std::endl;
		std::cout << u8"Elapsed time since launch: " << elapsed << "s\n" << std::endl;
		std::cout << u8"[Missile] ���� �浹 �ð� �ʰ� - ����!\n";
		missile_->MissileState = 4; // 4���� ���� ���¶�� ����
		running_ = false;
		return;
	}
}

// [�߰�]
void MissileController::setTargetAircraftId(const std::string& id) {
	targetAircraftId_ = id;
}

void MissileController::setAircraftMap(const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* aircraftMap) {
	aircraftMap_ = aircraftMap;
}

double MissileController::toRad(double deg) {
	return deg * M_PI / 180.0;
}

// ���浵 ���� �Ÿ� (km)
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