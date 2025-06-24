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

    MFR::MFR(const std::string& udpIp, int udpPort) : stopFlag_(false) {  // 초기화 명시 
        // === WSA 초기화 ===
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }

        // === UDP 소켓 생성 ===
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("socket() failed");
        }

        // === 수신 바인드 ===
        if (!setupSocket(udpIp, udpPort)) {
            closesocket(sock_);
            WSACleanup();
            throw std::runtime_error("bind() failed");
        }

        // === 송신 주소 설정도 동일 IP/Port 사용 ===
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
        // 소켓 옵션 설정: SO_REUSEADDR (멀티캐스트 수신 가능하게)
        int reuse = 1;
        if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] setsockopt(SO_REUSEADDR) 실패: " << WSAGetLastError() << "\n";
            return false;
        }

        // 바인딩
        sockaddr_in recvAddr{};
        recvAddr.sin_family = AF_INET;
        recvAddr.sin_port = htons(udpPort);
        recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // ← 중요! 멀티캐스트는 INADDR_ANY로 바인딩해야 수신 가능

        if (bind(sock_, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] bind() 실패: " << WSAGetLastError() << "\n";
            return false;
        }

        // 멀티캐스트 그룹 가입
        ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = inet_addr(udpIp.c_str());    // 멀티캐스트 주소
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);           // 로컬 인터페이스

        if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] 멀티캐스트 그룹 가입 실패: " << WSAGetLastError() << "\n";
            return false;
        }

        // 멀티캐스트 송신 인터페이스 설정
        in_addr localInterface{};
        localInterface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_IF,
            (char*)&localInterface, sizeof(localInterface)) == SOCKET_ERROR) {
            std::cerr << u8"[MFR] 멀티캐스트 송신 인터페이스 설정 실패: " << WSAGetLastError() << "\n";
            return false;
        }

        std::cout << u8"[MFR] 멀티캐스트 수신 소켓 설정 완료 (" << udpIp << ":" << udpPort << ")\n";
        return true;
    }

    // 고도를 반영한 하버사인 공식
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

    // 부채꼴 함수, fov_deg 에 각도 지정
    bool MFR::isTargetInFOV(double lat1, double lon1,
        double lat2, double lon2,
        double heading_deg, double fov_deg)
    {
        // 위경도 차이를 거리로 근사 (단위: km)
        double dx = (lon2 - lon1) * EARTH_RADIUS_KM * DEG_TO_RAD * std::cos(lat1 * DEG_TO_RAD);
        double dy = (lat2 - lat1) * EARTH_RADIUS_KM * DEG_TO_RAD;

        // 타겟 방향의 방위각 계산 (0도 = 북쪽, 시계방향 기준)
        double targetBearing = std::atan2(dx, dy) * 180.0 / PI;
        if (targetBearing < 0) targetBearing += 360.0;

        // 방위각 차이 계산
        double diff = std::fabs(targetBearing - heading_deg);
        if (diff > 180.0) diff = 360.0 - diff;

        // 시야각 내에 있는지 여부
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

            if (received == SOCKET_ERROR) {  // 41 바이트 이외 패킷은 전부 무시
                continue;
            }

            if (received == sizeof(packet)) {
                std::string id(packet.aircraftId, 8);
                aircraftMap[id] = packet;

                double dist = computeDistanceKm(batteryLat, batteryLon, batteryAlt,
                    packet.latitude, packet.longitude, packet.altitude);

                //std::cout << u8"[MFR] 수신 패킷 ← "
                //    << u8"ID: " << id
                //    << u8", 위도: " << packet.latitude
                //    << u8", 경도: " << packet.longitude
                //    << u8", 고도: " << packet.altitude
                //    << u8", 이벤트: " << packet.eventCode
                //    << u8", 거리: " << dist << u8"km\n";
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
                        //std::cout << u8"[MFR] 추적 탐지 전송 → ID: " << id << u8", 거리: " << dist << "km\n";
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
                        //std::cout << u8"[MFR] 광역 탐지 전송 → ID: " << id << u8", 거리: " << dist << "km\n";
                        sendto(sock_, reinterpret_cast<char*>(&p), sizeof(p), 0,
                            (sockaddr*)&sendAddr_, sizeof(sendAddr_));
                    }
                }
                lastWideSend = now;
            }
        }

        std::cout << u8"[MFR] 탐지 스레드 정상 종료됨\n";
    }

}