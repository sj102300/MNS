#include "AircraftCoordinate.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <thread>
#include <mutex>

#define x first
#define y second

std::mutex mtx;

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

    // === 루프백 허용 ===
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
        double xBits = currentPoint.x;
        double yBits = currentPoint.y;
        double altitude = 10;

        memcpy(buffer, &eventCode, 4);
        memcpy(buffer + 4, &bodyLength, 4);
        memcpy(buffer + 8, id.c_str(), 8);
        memcpy(buffer + 16, &xBits, sizeof(double));
        memcpy(buffer + 24, &yBits, sizeof(double));
        memcpy(buffer + 32, &altitude, sizeof(double));
        buffer[40] = IFF;
    }

    int sendSize = sendto(udpSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&multicastAddr, sizeof(multicastAddr));
    if (sendSize < 0) {
        std::cerr << "Failed to send data. Error: " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "[전송 성공] ID: " << id
            << " | 위도(Lat): " << currentPoint.x
            << " | 경도(Lon): " << currentPoint.y
            << " | IFF: " << IFF << std::endl;
    }
}
