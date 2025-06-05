#define _CRT_SECURE_NO_WARNINGS
#include "sender.h"

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
    strncpy(packet.EventId, eventStr.c_str(), sizeof(packet.EventId));
    strncpy(packet.MissileId, missileId.c_str(), sizeof(packet.MissileId));
    strncpy(packet.AirCraftId, aircraftId.c_str(), sizeof(packet.AirCraftId));

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
