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

    multicastAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "239.0.0.2", &multicastAddr.sin_addr) <= 0) {
        std::cerr << "Invalid multicast address" << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }
    multicastAddr.sin_port = htons(9999);

    int ttl = 1;
    if (setsockopt(udpSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << "Failed to set socket options. Error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }
}

void AircraftCoordinate::sendAircraftInfo(std::pair<double, double> currentPoint, std::string id, char IFF) {
    char buffer[sizeof(char) * 8 + sizeof(double) * 3 + sizeof(char)];
    {
        std::lock_guard<std::mutex> lock(mtx);

        uint64_t xBits = *reinterpret_cast<uint64_t*>(&currentPoint.x);
        uint64_t yBits = *reinterpret_cast<uint64_t*>(&currentPoint.y);
        double altitude = 10.0;

        memcpy(buffer, id.c_str(), 8);
        memcpy(buffer + 8, &xBits, sizeof(uint64_t));
        memcpy(buffer + 8 + sizeof(uint64_t), &yBits, sizeof(uint64_t));
        memcpy(buffer + 8 + sizeof(uint64_t) * 2, &altitude, sizeof(double));
        buffer[8 + sizeof(uint64_t) * 3] = IFF;
    }

    int sendSize = sendto(udpSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&multicastAddr, sizeof(multicastAddr));
    if (sendSize < 0) {
        std::cerr << "Failed to send data. Error: " << WSAGetLastError() << std::endl;
    }
    std::cout << "통신 성공" << std::endl;
}
