#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "MFR.h"
#include <iostream>
#include <cmath>
namespace mfr {

    constexpr double PI = 3.141592653589793;
    constexpr double EARTH_RADIUS_KM = 6371.0;

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

            std::cout << u8"[MFR] 탐지 루프 시작됨. stopFlag = " << stopFlag_ << "\n";
            if (received == SOCKET_ERROR) {
                std::cerr << u8"[MFR] recvfrom() 실패: " << WSAGetLastError() << "\n";
                continue;
            }


            if (received == sizeof(packet)) {
                // 수신 패킷 정보 출력
                std::cout << u8"[MFR] 수신 패킷 ← "
                    << u8"ID: " << packet.aircraftId
                    << u8", 위도: " << packet.latitude
                    << u8", 경도: " << packet.longitude
                    << u8", 고도: " << packet.altitude
                    << u8", 이벤트: " << packet.eventCode << "\n";
                std::cout << u8"====================================================\n";

                if (packet.eventCode == 1001 &&
                    withinRange(packet.latitude, packet.longitude, packet.altitude,
                        batteryLat, batteryLon, batteryAlt)) {

                    packet.eventCode = 1002;

                    // 전송 전 패킷 정보 출력
                    std::cout << u8"[MFR] 전송 패킷 → "
                        << u8"ID: " << packet.aircraftId
                        << u8", 위도: " << packet.latitude
                        << u8", 경도: " << packet.longitude
                        << u8", 고도: " << packet.altitude
                        << u8", 이벤트: " << packet.eventCode << "\n";
                    std::cout << u8"====================================================\n";

                    sendto(sock_, reinterpret_cast<char*>(&packet), sizeof(packet), 0,
                        (sockaddr*)&sendAddr_, sizeof(sendAddr_));
                }
            }
        }

        std::cout << u8"[MFR] 탐지 스레드 정상 종료됨\n";
    }

}