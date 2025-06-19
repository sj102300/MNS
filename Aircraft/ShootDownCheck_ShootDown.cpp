#include "ShootDown.h"
#define x first
#define y second

double toRadians(double degree) {
    return degree * (pi / 180);
}

// 두 위도/경도 좌표 간의 거리를 계산하는 함수
double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    double phi1 = toRadians(lat1);
    double phi2 = toRadians(lat2);
    double deltaPhi = toRadians(lat2 - lat1);
    double deltaLambda = toRadians(lon2 - lon1);

    double a = std::sin(deltaPhi / 2) * std::sin(deltaPhi / 2) +
        std::cos(phi1) * std::cos(phi2) *
        std::sin(deltaLambda / 2) * std::sin(deltaLambda / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    double distance = EarthR * c;
    return distance;
}

//오차범위 30m
bool ShootDowns(std::pair<double,double> currentPoint, std::pair<double,double> missilePoint) {
    double Adistance = haversineDistance(currentPoint.x, currentPoint.y, missilePoint.x, missilePoint.y);
    if (Adistance <= 0.15) return true; //1km 반경 격추 성공 판단
	return false;
}