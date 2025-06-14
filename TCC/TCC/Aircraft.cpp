#include "Aircraft.h"
#include <iostream>

Aircraft::Aircraft(std::string aircraftId, TCC::Position pos, bool isEnemy__) : aircraftId_(aircraftId), pos_(pos), 
    isEnemy_(isEnemy__), dirVec_({ 0,0 }), status_(EngagementStatus::NotEngageable), impactPoint_({ -200, -200, 10 }) {}

Aircraft::~Aircraft() {
    std::cout<<"aircraft deleted"<<std::endl;
}

const bool Aircraft::isEnemy() const {
	return isEnemy_;
}

void Aircraft::updatePosition(TCC::Position& newLocation) {
	//std::cout << "Aircraft::updatePosition() called" << std::endl;
	calcDirVec(newLocation);
    pos_ = newLocation;
}

namespace {
    constexpr double PI = 3.14159265358979323846;
    constexpr double EARTH_RADIUS_KM = 6371.0;
    constexpr double DEG_TO_RAD = PI / 180.0;
    constexpr double RAD_TO_DEG = 180.0 / PI;

    // 위경도 → 평면 좌표 변환
    void latLonToXY(const TCC::Position& origin, const TCC::Position& target, double& x, double& y) {
        double dLat = (target.latitude_ - origin.latitude_) * DEG_TO_RAD;
        double dLon = (target.longitude_ - origin.longitude_) * DEG_TO_RAD;
        double latRad = origin.latitude_ * DEG_TO_RAD;

        x = EARTH_RADIUS_KM * dLon * cos(latRad);
        y = EARTH_RADIUS_KM * dLat;
    }

    // 평면 좌표 → 위경도 변환
    TCC::Position xyToLatLon(const TCC::Position& origin, double x, double y, double altitude) {
        double lat = origin.latitude_ + (y / EARTH_RADIUS_KM) * RAD_TO_DEG;
        double lon = origin.longitude_ + (x / (EARTH_RADIUS_KM * cos(origin.latitude_ * DEG_TO_RAD))) * RAD_TO_DEG;
        return { lat, lon, altitude };
    }
}

void Aircraft::calcDirVec(const TCC::Position& newPos) {
    // 항공기 위치 - 새로운 위치 (방향 벡터 계산)
    double dx = newPos.longitude_ - pos_.longitude_;
    double dy = newPos.latitude_ - pos_.latitude_;

    // 벡터 크기 계산 (정규화 필요)
    double magnitude = std::sqrt(dx * dx + dy * dy);
    if (magnitude == 0.0) return; // 방향 없음

    // 정규화
    dirVec_.dx_ = dx / magnitude;
    dirVec_.dy_ = dy / magnitude;

    return;
}

void Aircraft::getImpactPoint(TCC::Position &impactPoint) {
    impactPoint = impactPoint_;
}

bool Aircraft::calcImpactPoint(TCC::Position& batteryLoc) {
    double vt = 1.0; // 항공기 속도 (km/s)
    double vm = 2.0; // 미사일 속도 (km/s)

    if (dirVec_.isZeroVector()) {
        impactPoint_ = { -200, -200, 10 }; // 유효하지 않은 값
		std::cout << "impossible to intercept: zero direction vector" << std::endl;
        return false;
    }

    // 1. 위치 평면 좌표로 변환
    double tx, ty, mx, my;
    latLonToXY(batteryLoc, pos_, tx, ty);
    latLonToXY(batteryLoc, batteryLoc, mx, my); // = 0,0

    // 3. 이차방정식으로 요격 시간 t 계산
    double a = vt * vt - vm * vm;
    double b = 2 * vt * ((tx - mx) * dirVec_.dx_ + (ty - my) * dirVec_.dy_);
    double c = (tx - mx) * (tx - mx) + (ty - my) * (ty - my);
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
		std::cout << "impossible to intercept: discriminant < 0" << std::endl;
        impactPoint_ = { -200, -200, 10 };   // 수학적으로 요격 불가능
        return false;
    }

    double sqrtD = std::sqrt(discriminant);
    double t1 = (-b + sqrtD) / (2 * a);
    double t2 = (-b - sqrtD) / (2 * a);
    double t = (t1 > 0) ? t1 : ((t2 > 0) ? t2 : -1);
    if (t < 0) {
		std::cout << "impossible to intercept: t < 0" << std::endl;
        impactPoint_ = { -200, -200, 10 };       //과거에 요격했어야함.
        return false;
    }

    // 4. Impact point 계산 (평면 x,y)
    double impact_x = tx + vt * t * dirVec_.dx_;
    double impact_y = ty + vt * t * dirVec_.dy_;

    // 5. 위경도로 되돌리기
    impactPoint_ = xyToLatLon(batteryLoc, impact_x, impact_y, pos_.altitude_);

    return true;
}

//return value: NotEngagable -> Engageable 상태로 변화했으면 true. EngagementManager에게 알려주어야 할 경우에만 push
//교전 가능 범위내로 들어왔는지만 판단.
bool Aircraft::hasBecomeEngageable(TCC::Position &batteryLoc, unsigned int& engagementStatus) {

    engagementStatus = (unsigned int)status_;
    if (status_ == EngagementStatus::Engaging || status_ == EngagementStatus::Destroyed) {
        return false; // 이미 교전 중이거나 격추된 상태
    }

    // 위도와 경도를 라디안으로 변환
    double lat1 = batteryLoc.latitude_ * DEG_TO_RAD;
    double lon1 = batteryLoc.longitude_ * DEG_TO_RAD;
    double lat2 = pos_.latitude_ * DEG_TO_RAD;
    double lon2 = pos_.longitude_ * DEG_TO_RAD;

    // Haversine 거리 계산
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    double distanceKm = EARTH_RADIUS_KM * c;

    // 100km 이내, 교전 가능
    if (distanceKm <= 130.0) {
        if (status_ == EngagementStatus::NotEngageable) {       //교전 불가능이었다가 교전 가능범위 내로 진입한 상태
            status_ = EngagementStatus::Engageable;
			engagementStatus = (unsigned int)status_;
			std::cout << "Aircraft has become engageable: " << aircraftId_ << std::endl;
			return true; // 교전 가능 상태로 변경됨
        }   
        else {             // 이미 교전 가능 상태이므로 변경 없음
            status_ = EngagementStatus::Engageable;
            engagementStatus = (unsigned int)status_;
			//std::cout << "Aircraft is already engageable: " << aircraftId_ << std::endl;
            return false;
        }
     }

    status_ = EngagementStatus::NotEngageable;
    engagementStatus = (unsigned int)status_;
    return false;
}

bool Aircraft::isEngagable() {
    return status_ == EngagementStatus::Engageable || status_ == EngagementStatus::Engaging;
}

unsigned int Aircraft::updateStatus(unsigned int engagementStatus) {
    return status_ = engagementStatus;
}