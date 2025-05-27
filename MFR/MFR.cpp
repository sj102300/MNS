#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "MFR.h"
#include <iostream>
#include <cmath>
namespace mfr {

    constexpr double PI = 3.141592653589793;
    constexpr double EARTH_RADIUS_KM = 6371.0;

    MFR::MFR(const std::string& udpIp, int udpPort) : stopFlag_(false) {  // �ʱ�ȭ ��� 
        // === WSA �ʱ�ȭ ===
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }

        // === UDP ���� ���� ===
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("socket() failed");
        }

        // === ���� ���ε� ===
        if (!setupSocket(udpIp, udpPort)) {
            closesocket(sock_);
            WSACleanup();
            throw std::runtime_error("bind() failed");
        }

        // === �۽� �ּ� ������ ���� IP/Port ��� ===
        std::memset(&sendAddr_, 0, sizeof(sendAddr_));
        sendAddr_.sin_family = AF_INET;
        sendAddr_.sin_port = htons(udpPort);
        sendAddr_.sin_addr.s_addr = inet_addr(udpIp.c_str());
    }

    MFR::~MFR() {
        closesocket(sock_);
        WSACleanup();
    }

    bool MFR::setupSocket(const std::string& udpIp, int udpPort) {
        // ���� �ɼ� ����: SO_REUSEADDR (��Ƽĳ��Ʈ ���� �����ϰ�)
        int reuse = 1;
        if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] setsockopt(SO_REUSEADDR) ����: " << WSAGetLastError() << "\n";
            return false;
        }

        // ���ε�
        sockaddr_in recvAddr{};
        recvAddr.sin_family = AF_INET;
        recvAddr.sin_port = htons(udpPort);
        recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // �� �߿�! ��Ƽĳ��Ʈ�� INADDR_ANY�� ���ε��ؾ� ���� ����

        if (bind(sock_, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] bind() ����: " << WSAGetLastError() << "\n";
            return false;
        }

        // ��Ƽĳ��Ʈ �׷� ����
        ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = inet_addr(udpIp.c_str());    // ��Ƽĳ��Ʈ �ּ�
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);           // ���� �������̽�

        if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] ��Ƽĳ��Ʈ �׷� ���� ����: " << WSAGetLastError() << "\n";
            return false;
        }

        // ��Ƽĳ��Ʈ �۽� �������̽� ����
        in_addr localInterface{};
        localInterface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_IF,
            (char*)&localInterface, sizeof(localInterface)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] ��Ƽĳ��Ʈ �۽� �������̽� ���� ����: " << WSAGetLastError() << "\n";
            return false;
        }

        std::cout << u8"[MFR] ��Ƽĳ��Ʈ ���� ���� ���� �Ϸ� (" << udpIp << ":" << udpPort << ")\n";
        return true;
    }


    bool MFR::withinRange(double lat, double lon, double alt,
        double batteryLat, double batteryLon, double batteryAlt) {
        double dLat = (lat - batteryLat) * PI / 180.0;
        double dLon = (lon - batteryLon) * PI / 180.0;

        double lat1 = batteryLat * PI / 180.0;
        double lat2 = lat * PI / 180.0;

        double a = std::pow(std::sin(dLat / 2), 2) +
            std::cos(lat1) * std::cos(lat2) * std::pow(std::sin(dLon / 2), 2);
        double c = 2 * std::asin(std::sqrt(a));

        double groundDistance = EARTH_RADIUS_KM * c;
        double dAlt = (alt - batteryAlt) / 1000.0;
        double totalDistance = std::sqrt(groundDistance * groundDistance + dAlt * dAlt);

        return totalDistance <= maxDetectKm;
    }

    void MFR::stop() {
        stopFlag_ = true;
    }

    void MFR::run(double batteryLat, double batteryLon, double batteryAlt) {
        sockaddr_in senderAddr{};
        int addrLen = sizeof(senderAddr);
        AircraftPacket packet;

        while (!stopFlag_) {
            int received = recvfrom(sock_, reinterpret_cast<char*>(&packet), sizeof(packet), 0,
                (sockaddr*)&senderAddr, &addrLen);

            std::cout << u8"[MFR] Ž�� ���� ���۵�. stopFlag = " << stopFlag_ << "\n";
            if (received == SOCKET_ERROR) {
                std::cerr << u8"[MFR] recvfrom() ����: " << WSAGetLastError() << "\n";
                continue;
            }


            if (received == sizeof(packet)) {
                // ���� ��Ŷ ���� ���
                std::cout << u8"[MFR] ���� ��Ŷ �� "
                    << u8"ID: " << packet.aircraftId
                    << u8", ����: " << packet.latitude
                    << u8", �浵: " << packet.longitude
                    << u8", ��: " << packet.altitude
                    << u8", �̺�Ʈ: " << packet.eventCode << "\n";
                std::cout << u8"====================================================\n";

                if (packet.eventCode == 1001 &&
                    withinRange(packet.latitude, packet.longitude, packet.altitude,
                        batteryLat, batteryLon, batteryAlt)) {

                    packet.eventCode = 1002;

                    // ���� �� ��Ŷ ���� ���
                    std::cout << u8"[MFR] ���� ��Ŷ �� "
                        << u8"ID: " << packet.aircraftId
                        << u8", ����: " << packet.latitude
                        << u8", �浵: " << packet.longitude
                        << u8", ��: " << packet.altitude
                        << u8", �̺�Ʈ: " << packet.eventCode << "\n";
                    std::cout << u8"====================================================\n";

                    sendto(sock_, reinterpret_cast<char*>(&packet), sizeof(packet), 0,
                        (sockaddr*)&sendAddr_, sizeof(sendAddr_));
                }
            }
        }

        std::cout << u8"[MFR] Ž�� ������ ���� �����\n";
    }

}