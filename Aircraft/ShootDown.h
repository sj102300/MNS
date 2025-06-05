#pragma once
#include <utility>
#include <cmath>

// 지구 반경 (km)
constexpr double EarthR = 6356.784;
constexpr double pi = 3.14159265358979;

// 도(degree)를 라디안(radian)으로 변환
double toRadians(double degree);

// 두 위도/경도 좌표 간 거리(km) 계산 (Haversine 공식)
double haversineDistance(double lat1, double lon1, double lat2, double lon2);

// 두 좌표 간 거리가 30m(0.03km) 이내면 true 반환
bool ShootDowns(std::pair<double, double> currentPoint, std::pair<double, double> missilePoint);
