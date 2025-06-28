#pragma once

#include "Missile.h"
#include "Aircraft.h"  // [추가] Aircraft
#include <chrono>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <cmath>
#include <unordered_set>
#include "DestroyedAircrafts.h"
#define EARTH_RADIUS_KM 6371.0

class Aircraft;  // [추가] 선언

class MissileController {
public:
	enum class MovementState {
		Straight,
		Turning
	};
	void setMovementState(MovementState state);
	MovementState getMovementState() const;
	void updateTurnToNewTarget(float speed = 5.0f);
	void forceTurnToNewTarget();  // [추가] 새로운 곡선 함수 실행

	MissileController(DestroyedAircraftsTracker * tracker);

	void setMissile(std::shared_ptr<Missile> m);

	void setTarget(Location pos);  // 기존 임팩트 포인트용

	void start(float speed);

	void stop();

	void updateLoop(float speed);

	void updatePosition(float speed = 5.0f);

	// [추가] 추적용 항공기 ID, 맵 설정
	void setTargetAircraftId(const std::string& aircraftId);
	void setAircraftMap(const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* aircraftMap);

	double toRad(double deg);
	double haversine(double lat1, double lon1, double lat2, double lon2);

	// gtest를 위한 코드 2줄
	bool hasTarget() const { return hasTarget_; }
	double getEstimatedTimeToImpact() const { return estimatedTimeToImpact_; }

private:
	int state7Count_ = 0;
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

	// [추가]
	std::string targetAircraftId_;
	const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* aircraftMap_ = nullptr;
	bool hasEnteredTerminalGuidance_ = false;

	DestroyedAircraftsTracker* tracker_;

	MovementState currentMovementState_ = MovementState::Straight;  // 현재 이동 상태
	double turning_heading_ = 0.0;
	double turning_w_ = 0.0;
	bool turning_initialized_ = false;  // 회전 초기화 여부
	double compute_heading(double from_lat, double from_lon, double to_lat, double to_lon);
	double Cal_RateOfTurn(double current_w, double heading_error, double dt);
	std::pair<double, double> update_latlon(double heading, double dt, double lat_deg, double lon_deg);
	bool isTurningToNewTarget_ = false;  // [추가] 새로운 곡선 함수 실행 여부
};