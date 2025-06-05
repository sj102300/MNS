#pragma once
#include <utility>
#include <cmath>

// ���� �ݰ� (km)
constexpr double EarthR = 6356.784;
constexpr double pi = 3.14159265358979;

// ��(degree)�� ����(radian)���� ��ȯ
double toRadians(double degree);

// �� ����/�浵 ��ǥ �� �Ÿ�(km) ��� (Haversine ����)
double haversineDistance(double lat1, double lon1, double lat2, double lon2);

// �� ��ǥ �� �Ÿ��� 30m(0.03km) �̳��� true ��ȯ
bool ShootDowns(std::pair<double, double> currentPoint, std::pair<double, double> missilePoint);
