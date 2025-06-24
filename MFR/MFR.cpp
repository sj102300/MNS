#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "MFR.h"
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <chrono>

namespace mfr {

    constexpr double PI = 3.141592653589793;
    constexpr double EARTH_RADIUS_KM = 6371.0;
    constexpr double DEG_TO_RAD = PI / 180.0;

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

    // ���� �ݿ��� �Ϲ����� ����
    double MFR::computeDistanceKm(double lat1, double lon1, double alt1,
        double lat2, double lon2, double alt2) {
        double dLat = (lat2 - lat1) * PI / 180.0;
        double dLon = (lon2 - lon1) * PI / 180.0;

        lat1 *= PI / 180.0;
        lat2 *= PI / 180.0;

        double a = std::pow(std::sin(dLat / 2), 2) +
            std::cos(lat1) * std::cos(lat2) * std::pow(std::sin(dLon / 2), 2);
        double c = 2 * std::asin(std::sqrt(a));

        double groundDistance = EARTH_RADIUS_KM * c;
        double dAlt = (alt2 - alt1) / 1000.0;
        return std::sqrt(groundDistance * groundDistance + dAlt * dAlt);
    }

    // ��ä�� �Լ�, fov_deg �� ���� ����
    bool MFR::isTargetInFOV(double lat1, double lon1,
        double lat2, double lon2,
        double heading_deg, double fov_deg)
    {
        // ���浵 ���̸� �Ÿ��� �ٻ� (����: km)
        double dx = (lon2 - lon1) * EARTH_RADIUS_KM * DEG_TO_RAD * std::cos(lat1 * DEG_TO_RAD);
        double dy = (lat2 - lat1) * EARTH_RADIUS_KM * DEG_TO_RAD;

        // Ÿ�� ������ ������ ��� (0�� = ����, �ð���� ����)
        double targetBearing = std::atan2(dx, dy) * 180.0 / PI;
        if (targetBearing < 0) targetBearing += 360.0;

        // ������ ���� ���
        double diff = std::fabs(targetBearing - heading_deg);
        if (diff > 180.0) diff = 360.0 - diff;

        // �þ߰� ���� �ִ��� ����
        return diff <= fov_deg / 2.0;
    }

    void MFR::stop() {
        stopFlag_ = true;
    }

    void MFR::run(double batteryLat, double batteryLon, double batteryAlt) {
        sockaddr_in senderAddr{};
        int addrLen = sizeof(senderAddr);
        AircraftPacket packet;

        std::unordered_map<std::string, AircraftPacket> aircraftMap;
        auto lastTrackSend = std::chrono::steady_clock::now();
        auto lastWideSend = std::chrono::steady_clock::now();

        while (!stopFlag_) {
            int received = recvfrom(sock_, reinterpret_cast<char*>(&packet), sizeof(packet), 0,
                (sockaddr*)&senderAddr, &addrLen);

            if (received == SOCKET_ERROR) {  // 41 ����Ʈ �̿� ��Ŷ�� ���� ����
                continue;
            }

            if (received == sizeof(packet)) {
                std::string id(packet.aircraftId, 8);
                aircraftMap[id] = packet;

                double dist = computeDistanceKm(batteryLat, batteryLon, batteryAlt,
                    packet.latitude, packet.longitude, packet.altitude);

                //std::cout << u8"[MFR] ���� ��Ŷ �� "
                //    << u8"ID: " << id
                //    << u8", ����: " << packet.latitude
                //    << u8", �浵: " << packet.longitude
                //    << u8", ��: " << packet.altitude
                //    << u8", �̺�Ʈ: " << packet.eventCode
                //    << u8", �Ÿ�: " << dist << u8"km\n";
                //std::cout << u8"\n========================================================================================================\n";
            }

            auto now = std::chrono::steady_clock::now();

            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTrackSend).count() >= 100) {
                for (const auto& entry : aircraftMap) {
                    const std::string& id = entry.first;
                    const AircraftPacket& pkt = entry.second;

                    double dist = computeDistanceKm(batteryLat, batteryLon, batteryAlt,
                        pkt.latitude, pkt.longitude, pkt.altitude);
                    if (dist <= 150.0 && pkt.eventCode == 1001) {
                        AircraftPacket p = pkt;
                        p.eventCode = 1002;
                        //std::cout << u8"[MFR] ���� Ž�� ���� �� ID: " << id << u8", �Ÿ�: " << dist << "km\n";
                        sendto(sock_, reinterpret_cast<char*>(&p), sizeof(p), 0,
                            (sockaddr*)&sendAddr_, sizeof(sendAddr_));
                    }
                }
                lastTrackSend = now;
            }

            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWideSend).count() >= 1000) {
                for (const auto& entry : aircraftMap) {
                    const std::string& id = entry.first;
                    const AircraftPacket& pkt = entry.second;

                    double dist = computeDistanceKm(batteryLat, batteryLon, batteryAlt,
                        pkt.latitude, pkt.longitude, pkt.altitude);
                    if (dist > 150.0 && dist <= 300.0 && pkt.eventCode == 1001) {
                        AircraftPacket p = pkt;
                        p.eventCode = 1002;
                        //std::cout << u8"[MFR] ���� Ž�� ���� �� ID: " << id << u8", �Ÿ�: " << dist << "km\n";
                        sendto(sock_, reinterpret_cast<char*>(&p), sizeof(p), 0,
                            (sockaddr*)&sendAddr_, sizeof(sendAddr_));
                    }
                }
                lastWideSend = now;
            }
        }

        std::cout << u8"[MFR] Ž�� ������ ���� �����\n";
    }

}