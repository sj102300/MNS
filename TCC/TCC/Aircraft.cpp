#include "Aircraft.h"

Aircraft::Aircraft(std::string aircraftId, TCC::Position pos, bool isEnemy__) : aircraftId_(aircraftId), pos_(pos), 
    isEnemy_(isEnemy__), dirVec_({ 0,0 }), status_(EngagementStatus::NotEngageable), impactPoint_({ -200, -200, 10 }) {}

#include <iostream>
Aircraft::~Aircraft() {
    std::cout<<"aircraft deleted"<<std::endl;
}
const bool Aircraft::isEnemy() const {
	return isEnemy_;
}

void Aircraft::updatePosition(TCC::Position& newLocation) {
	//std::cout << "Aircraft::updatePosition() called" << std::endl;
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
    double dx = pos_.longitude_ - newPos.longitude_;
    double dy = pos_.latitude_ - newPos.latitude_;

    // 벡터 크기 계산 (정규화 필요)
    double magnitude = std::sqrt(dx * dx + dy * dy);
    if (magnitude == 0.0) return; // 방향 없음

    // 정규화
    dirVec_.dx_ = dx / magnitude;
    dirVec_.dy_ = dy / magnitude;
    return;
}

TCC::Position Aircraft::getImpactPoint() {
    return impactPoint_;
}

void Aircraft::calcImpactPoint() {
    TCC::Position batteryLoc = { 37.5597, 126.9869, 10 };
    double vt = 1.0; // 항공기 속도 (km/s)
    double vm = 2.0; // 미사일 속도 (km/s)

    if (dirVec_.isZeroVector()) {
        impactPoint_ = { -200, -200, 10 }; // 유효하지 않은 값
        return;
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
        impactPoint_ = { -200, -200, 10 };   // 수학적으로 요격 불가능
        return;
    }

    double sqrtD = std::sqrt(discriminant);
    double t1 = (-b + sqrtD) / (2 * a);
    double t2 = (-b - sqrtD) / (2 * a);
    double t = (t1 > 0) ? t1 : ((t2 > 0) ? t2 : -1);
    if (t < 0) {
        impactPoint_ = { -200, -200, 10 };       //과거에 요격했어야함.
        return;
    }

    // 4. Impact point 계산 (평면 x,y)
    double impact_x = tx + vt * t * dirVec_.dx_;
    double impact_y = ty + vt * t * dirVec_.dy_;

    // 5. 위경도로 되돌리기
    impactPoint_ = xyToLatLon(batteryLoc, impact_x, impact_y, pos_.altitude_);
}

bool Aircraft::isIpInEngageRange(unsigned int engagementStatus, TCC::Position& impactPoint, bool &isEngagementStatusChanged) {

    engagementStatus = (unsigned int)status_;

    if (status_ == EngagementStatus::Engaging || status_ == EngagementStatus::Destroyed) {
        return false;
    }

    calcImpactPoint();
    impactPoint = impactPoint_;

    // 포대 위치
    TCC::Position batteryLoc = { 37.5432, 126.7321, 10 };

    // 위도와 경도를 라디안으로 변환
    double lat1 = batteryLoc.latitude_ * DEG_TO_RAD;
    double lon1 = batteryLoc.longitude_ * DEG_TO_RAD;
    double lat2 = impactPoint_.latitude_ * DEG_TO_RAD;
    double lon2 = impactPoint_.longitude_ * DEG_TO_RAD;

    // Haversine 거리 계산
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    double distanceKm = EARTH_RADIUS_KM * c;

    // 100km 이내, 교전 가능
    if (distanceKm <= 100.0) {
        if (isStatusChanged(status_, EngagementStatus::Engageable)) {
            isEngagementStatusChanged = true;
        }
        status_ = EngagementStatus::Engageable;
        engagementStatus = (unsigned int) status_;
        return true;
    }

    status_ = EngagementStatus::NotEngageable;
    engagementStatus = (unsigned int)status_;
    return false;
}

bool Aircraft::isStatusChanged(unsigned int a, unsigned int b) {
    return a == b;
}

bool Aircraft::isEngagable() {
    return status_ == EngagementStatus::Engageable || status_ == EngagementStatus::Engaging;
}

unsigned int Aircraft::updateStatus(unsigned int engagementStatus) {
    return status_ = engagementStatus;
}