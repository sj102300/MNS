#include "AircraftCoordinate.h"
#include <cmath>
#include <thread>
// #include <iostream>

#define pi 3.14159265358979
#define EarthR 6356.784
#define x first
#define y second

double AircraftCoordinate::toRadians(double degree) {
    return degree * pi / 180.0;
}

double AircraftCoordinate::haversine(double lat1, double lon1, double lat2, double lon2) {
    lat1 = toRadians(lat1);
    lon1 = toRadians(lon1);
    lat2 = toRadians(lat2);
    lon2 = toRadians(lon2);

    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EarthR * c;
}

vector<double> AircraftCoordinate::makeStartOpt(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) {
    vector<double> startOpt;
    double total_distance = haversine(startPoint.x, startPoint.y, finishPoint.x, finishPoint.y);
    double unit_distance = 1.0 / 100.0;
    double count = total_distance / unit_distance;
    double dx = (finishPoint.x - startPoint.x) / count;
    double dy = (finishPoint.y - startPoint.y) / count;
    startOpt.push_back(dx);
    startOpt.push_back(dy);

    return startOpt;
}

pair<double, double> AircraftCoordinate::makePoint(double dx, double dy, pair<double, double> currentPoint) {
    /* 점검 코드
    std::cout << "dx : " << dx << " " << "dy : " << dy << std::endl;
    */
    currentPoint.x += dx;
    currentPoint.y += dy;

    return currentPoint;
}
