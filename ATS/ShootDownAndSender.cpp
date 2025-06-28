#include "ShootDownAndSender.h"
#include "InterceptResultPacket.h"  // 구조체 정의 포함

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

constexpr double EarthR = 6371.0;
constexpr double pi = 3.14159265358979;

double toRadians(double degree) {
    return degree * (pi / 180);
}

double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
    double phi1 = toRadians(lat1);
    double phi2 = toRadians(lat2);
    double deltaPhi = toRadians(lat2 - lat1);
    double deltaLambda = toRadians(lon2 - lon1);

    double a = std::sin(deltaPhi / 2) * std::sin(deltaPhi / 2) +
        std::cos(phi1) * std::cos(phi2) *
        std::sin(deltaLambda / 2) * std::sin(deltaLambda / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return EarthR * c;
}

bool ShootDowns(std::pair<double, double> currentPoint, std::pair<double, double> missilePoint) {
    double distance = haversineDistance(currentPoint.first, currentPoint.second, missilePoint.first, missilePoint.second);
    return distance <= 0.2;
}

// === 전역 송신 소켓 ===
SOCKET udpSocket2 = INVALID_SOCKET;
sockaddr_in multicastAddr2;

void initializeMultiSenderSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return;
    }

    udpSocket2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket2 == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    multicastAddr2.sin_family = AF_INET;
    if (inet_pton(AF_INET, "239.0.0.1", &multicastAddr2.sin_addr) <= 0) {
        std::cerr << "Invalid multicast address" << std::endl;
        closesocket(udpSocket2);
        WSACleanup();
        return;
    }
    multicastAddr2.sin_port = htons(9000);

    int ttl = 1;
    if (setsockopt(udpSocket2, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << "Failed to set socket options. Error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket2);
        WSACleanup();
        return;
    }
}

void sendSuccessInfo(std::string aircraftId, std::string missileId) {
    std::cout << "격추성공: 항공기 ID " << aircraftId << ", 미사일 ID " << missileId << std::endl;

    InterceptResultPacket packet{};
    packet.EventCode = 2003;
    packet.BodyLength = sizeof(packet.EventId) + sizeof(packet.MissileId) + sizeof(packet.AirCraftId);

    std::string eventStr = "SUCCESS";
    memset(packet.EventId, 0, sizeof(packet.EventId));
    memcpy(packet.EventId, eventStr.c_str(), eventStr.length());

    // ID는 정확히 8바이트 복사 (null 없음)
    memcpy(packet.AirCraftId, aircraftId.data(), sizeof(packet.AirCraftId));
    memcpy(packet.MissileId, missileId.data(), sizeof(packet.MissileId));

    int sendSize = sendto(
        udpSocket2,
        reinterpret_cast<const char*>(&packet),
        sizeof(packet),
        0,
        (SOCKADDR*)&multicastAddr2,
        sizeof(multicastAddr2)
    );

    if (sendSize < 0) {
        std::cerr << "Failed to send data. Error: " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "통신 성공" << std::endl;
    }
}
