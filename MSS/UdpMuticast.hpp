#pragma once
#include "Packet.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

class UdpSender1 {
public:
    UdpSender1() : sock_(INVALID_SOCKET), port_(0), running_(false) ,missile_(nullptr) {}
    ~UdpSender1() { close(); }

    // 멀티캐스트 대상 주소와 포트를 받아 초기화
    bool init(const std::string& multicast_address, int port) {
        port_ = port;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return false;
        }

        // 소켓 생성 (UDP)
        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "Failed to create socket\n";
            return false;
        }
 

        // 멀티캐스트 전송 시 TTL(Time-To-Live) 옵션 설정 (기본 1: 로컬 네트워크)
        int ttl = 1;
        if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
            std::cerr << "setsockopt(IP_MULTICAST_TTL) failed\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }

        // 대상 주소 설정
        memset(&destAddr_, 0, sizeof(destAddr_));
        destAddr_.sin_family = AF_INET;
        destAddr_.sin_port = htons(port_);
        // 멀티캐스트 주소 변환: InetPtonA 사용 (멀티바이트 문자열)
        if (InetPtonA(AF_INET, multicast_address.c_str(), &destAddr_.sin_addr) != 1) { // destAddr을 이제 멀티캐스트 주소로
            std::cerr << "Invalid multicast address format\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }

        std::cout << "UDP Sender initialized for multicast group " << multicast_address
            << " on port " << port_ << "\n";
        return true;
    }

    // 객체의 맞는 정보를 전달하기 위함
    void setMissile(std::shared_ptr<Missile> m) {
        missile_ = m;
    }

    // 전송 루프: 주기적으로 데이터를 보내는 예시
    void start() {
        running_ = true;
        sendThread_ = std::thread(&UdpSender1::run, this);
    }

    // 전송 루프 내용: 2.5초마다 데이터 전송
    void run() {
        while (running_) {
            if (missile_) {
                int sent = sendto(sock_, (char*)&missile_, sizeof(Missile), 0,
                    (sockaddr*)&destAddr_, sizeof(destAddr_));
                if (sent == SOCKET_ERROR) {
                    std::cerr << "sendto failed: " << WSAGetLastError() << "\n";
                }
                else {
                    std::cout << " <Udp Multicast success> \n\n";
                    std::cout << "Altitude: " << missile_->MissileLoc.altitude << "\n"
                        << "Latitude: " << missile_->MissileLoc.latitude << "\n"
                        << "Longitude: " << missile_->MissileLoc.longitude << "\n\n";

                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        }
    }


    // 종료 및 자원 해제
    void close() {
        running_ = false;
        if (sendThread_.joinable()) {
            sendThread_.join();
        }
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
        WSACleanup();
    }

private:
    SOCKET sock_;
    sockaddr_in destAddr_;  // 멀티캐스트 대상 주소 저장
    int port_;
    bool running_;
    std::thread sendThread_;
    std::shared_ptr<Missile> missile_;
};
