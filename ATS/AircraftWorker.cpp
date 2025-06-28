#include "AircraftWorker.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace ats;

constexpr double pi = 3.14159265358979;
constexpr double EarthR = 6371.0;

// === 내부 유틸 ===
static double toRadians(double degree) {
    return degree * (pi / 180.0);
}

static double haversine(double lat1, double lon1, double lat2, double lon2) {
    lat1 = toRadians(lat1); lon1 = toRadians(lon1);
    lat2 = toRadians(lat2); lon2 = toRadians(lon2);
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return EarthR * c;
}

AircraftWorker::AircraftWorker(AircraftInfo info, ShootDownThread* shootDownThread)
    : info_(info), running_(true), shootDownThread_(shootDownThread) {
}

void AircraftWorker::stop() {
    running_ = false;
}

void AircraftWorker::operator()() {
    std::vector<double> direction = makeDirection(info_.startPoint, info_.finishPoint);
    double dx = direction[0];
    double dy = direction[1];

    // UDP 소켓 초기화
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in multicastAddr{};
    multicastAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "239.0.0.1", &multicastAddr.sin_addr);
    multicastAddr.sin_port = htons(9000);

    while (running_) {
        info_.currentPoint = movePoint(info_.currentPoint, dx, dy);
        sendAircraftInfo(udpSocket, multicastAddr);

        if (shootDownThread_) {
            AircraftSnapshot snap{ info_.id, info_.currentPoint, info_.IFF };
            shootDownThread_->enqueue(snap);  // 격추 판단 요청
        }

        if (reachedDestination(info_.currentPoint, info_.finishPoint)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    closesocket(udpSocket);
    WSACleanup();
}

std::vector<double> AircraftWorker::makeDirection(std::pair<double, double> from, std::pair<double, double> to) {
    double dist = haversine(from.first, from.second, to.first, to.second);
    double step = 0.1;  // 이동 단위 (10m)
    int count = std::max(1, static_cast<int>(dist / step));
    double dx = (to.first - from.first) / count;
    double dy = (to.second - from.second) / count;
    return { dx, dy };
}

std::pair<double, double> AircraftWorker::movePoint(std::pair<double, double> current, double dx, double dy) {
    return { current.first + dx, current.second + dy };
}

bool AircraftWorker::reachedDestination(std::pair<double, double> current, std::pair<double, double> dest) {
    return haversine(current.first, current.second, dest.first, dest.second) <= 0.05;
}

void AircraftWorker::sendAircraftInfo(SOCKET udpSocket, const sockaddr_in& addr) {
    char buffer[41]{};
    uint32_t eventCode = 1001;
    uint32_t bodyLength = 33;
    double lat = info_.currentPoint.first;
    double lon = info_.currentPoint.second;
    double alt = 10.0;

    memcpy(buffer, &eventCode, 4);
    memcpy(buffer + 4, &bodyLength, 4);
    memcpy(buffer + 8, info_.id.c_str(), 8);
    memcpy(buffer + 16, &lat, sizeof(double));
    memcpy(buffer + 24, &lon, sizeof(double));
    memcpy(buffer + 32, &alt, sizeof(double));
    buffer[40] = info_.IFF;

    sendto(udpSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&addr, sizeof(addr));
}
