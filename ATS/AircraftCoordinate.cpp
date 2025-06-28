#pragma once

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "CoordinateGeneration.h"
#include <cmath>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

#undef byte

#define pi 3.14159265358979
#define EarthR 6371.0
#define x first
#define y second

class AircraftCoordinate : public CoordinateGeneration {
private:
    SOCKET udpSocket;
    sockaddr_in multicastAddr;

    double haversine(double lat1, double lon1, double lat2, double lon2);
    double toRadians(double degree);

public:
    std::vector<double> makeStartOpt(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) override;
    std::pair<double, double> makePoint(double dx, double dy, std::pair<double, double> currentPoint) override;
    void sendAircraftInfo(std::pair<double, double> currentPoint, std::string id, char IFF) override;
    bool SurviveStatus(std::pair<double, double> currentPoint, std::pair<double, double> endPoint) override;
    void initializeMultiSocket();
};

static std::mutex mtx;

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

std::vector<double> AircraftCoordinate::makeStartOpt(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) {
    std::vector<double> startOpt;
    double total_distance = haversine(startPoint.x, startPoint.y, finishPoint.x, finishPoint.y);
    double unit_distance = 1.0 / 10.0;
    double count = total_distance / unit_distance;
    double dx = (finishPoint.x - startPoint.x) / count;
    double dy = (finishPoint.y - startPoint.y) / count;
    startOpt.push_back(dx);
    startOpt.push_back(dy);
    return startOpt;
}

std::pair<double, double> AircraftCoordinate::makePoint(double dx, double dy, std::pair<double, double> currentPoint) {
    currentPoint.x += dx;
    currentPoint.y += dy;
    return currentPoint;
}

bool AircraftCoordinate::SurviveStatus(std::pair<double, double> currentPoint, std::pair<double, double> endPoint) {
    return haversine(currentPoint.x, currentPoint.y, endPoint.x, endPoint.y) <= 0.05;
}

void AircraftCoordinate::initializeMultiSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return;
    }

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    char loopch = 1;
    if (setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopch, sizeof(loopch)) == SOCKET_ERROR) {
        std::cerr << "루프백 설정 실패\n";
    }

    multicastAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "239.0.0.1", &multicastAddr.sin_addr) <= 0) {
        std::cerr << "Invalid multicast address" << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }
    multicastAddr.sin_port = htons(9000);

    int ttl = 1;
    if (setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << "Failed to set socket options. Error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }
}

void AircraftCoordinate::sendAircraftInfo(std::pair<double, double> currentPoint, std::string id, char IFF) {
    char buffer[41];
    {
        std::lock_guard<std::mutex> lock(mtx);

        uint32_t eventCode = 1001;
        uint32_t bodyLength = 33;
        double lat = currentPoint.x;
        double lon = currentPoint.y;
        double altitude = 10;

        memcpy(buffer, &eventCode, 4);
        memcpy(buffer + 4, &bodyLength, 4);
        memcpy(buffer + 8, id.c_str(), 8);
        memcpy(buffer + 16, &lat, sizeof(double));
        memcpy(buffer + 24, &lon, sizeof(double));
        memcpy(buffer + 32, &altitude, sizeof(double));
        buffer[40] = IFF;
    }

    int sendSize = sendto(udpSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&multicastAddr, sizeof(multicastAddr));
    if (sendSize < 0) {
        std::cerr << "Failed to send data. Error: " << WSAGetLastError() << std::endl;
    }
    /*else {
        std::cout << "[전송 성공] ID: " << id
            << " | 위도: " << currentPoint.x
            << " | 경도: " << currentPoint.y
            << " | IFF: " << IFF << std::endl;
    }*/
}
