#include "AircraftCoordinate.h"
#include <cmath>
#include <thread>

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

void AircraftCoordinate::makePoint(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) {
    std::pair<double, double> currentPoint;
    double total_distance = haversine(startPoint.x, startPoint.y, finishPoint.x, finishPoint.y);
    double unit_distance = 1.0 / 100.0;
    double count = total_distance / unit_distance;
    double dx = (finishPoint.x - startPoint.x) / count;
    double dy = (finishPoint.y - startPoint.y) / count;
    currentPoint.x = startPoint.x;
    currentPoint.y = startPoint.y;

    while (true) {
        currentPoint.x += dx;
        currentPoint.y += dy;

        double checkDistance = haversine(currentPoint.x, currentPoint.y, finishPoint.x, finishPoint.y);
        if (checkDistance <= unit_distance) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
