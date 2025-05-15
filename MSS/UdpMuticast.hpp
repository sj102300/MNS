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

    // ��Ƽĳ��Ʈ ��� �ּҿ� ��Ʈ�� �޾� �ʱ�ȭ
    bool init(const std::string& multicast_address, int port) {
        port_ = port;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return false;
        }

        // ���� ���� (UDP)
        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "Failed to create socket\n";
            return false;
        }
 

        // ��Ƽĳ��Ʈ ���� �� TTL(Time-To-Live) �ɼ� ���� (�⺻ 1: ���� ��Ʈ��ũ)
        int ttl = 1;
        if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
            std::cerr << "setsockopt(IP_MULTICAST_TTL) failed\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }

        // ��� �ּ� ����
        memset(&destAddr_, 0, sizeof(destAddr_));
        destAddr_.sin_family = AF_INET;
        destAddr_.sin_port = htons(port_);
        // ��Ƽĳ��Ʈ �ּ� ��ȯ: InetPtonA ��� (��Ƽ����Ʈ ���ڿ�)
        if (InetPtonA(AF_INET, multicast_address.c_str(), &destAddr_.sin_addr) != 1) { // destAddr�� ���� ��Ƽĳ��Ʈ �ּҷ�
            std::cerr << "Invalid multicast address format\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }

        std::cout << "UDP Sender initialized for multicast group " << multicast_address
            << " on port " << port_ << "\n";
        return true;
    }

    // ��ü�� �´� ������ �����ϱ� ����
    void setMissile(std::shared_ptr<Missile> m) {
        missile_ = m;
    }

    // ���� ����: �ֱ������� �����͸� ������ ����
    void start() {
        running_ = true;
        sendThread_ = std::thread(&UdpSender1::run, this);
    }

    // ���� ���� ����: 2.5�ʸ��� ������ ����
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


    // ���� �� �ڿ� ����
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
    sockaddr_in destAddr_;  // ��Ƽĳ��Ʈ ��� �ּ� ����
    int port_;
    bool running_;
    std::thread sendThread_;
    std::shared_ptr<Missile> missile_;
};
