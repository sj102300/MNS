#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Missile.h"
#include "UdpMuticast.h"
#include "Isender.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>

#pragma comment(lib, "ws2_32.lib")


UdpMulticast::UdpMulticast() : sock_(INVALID_SOCKET), port_(0), running_(false), missile_(nullptr) {}
//UdpMulticast::~UdpMulticast() { close(); }

    // ��Ƽĳ��Ʈ ��� �ּҿ� ��Ʈ�� �޾� �ʱ�ȭ
bool UdpMulticast::init(const std::string& multicast_address, int port) {
        port_ = port;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return false;
        }

        // ���� ���� (UDP)
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "Failed to create socket\n";
            return false;
        }

        // ��� �ּ� ����
       // memset(&destAddr_, 0, sizeof(destAddr_));
        destAddr_.sin_family = AF_INET;
        destAddr_.sin_port = htons(port_);


        if (inet_pton(AF_INET, "239.0.0.1", &destAddr_.sin_addr) <= 0) { // destAddr�� ���� ��Ƽĳ��Ʈ �ּҷ�
            std::cerr << u8"Invalid multicast address format\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }

        in_addr localInterface{};
        if (InetPton(AF_INET, L"192.168.2.22", &localInterface) != 1) {
            std::cerr << u8"Failed to convert local interface IP\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }

        if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localInterface, sizeof(localInterface)) == SOCKET_ERROR) {
            std::cerr << u8"setsockopt(IP_MULTICAST_IF) failed: " << WSAGetLastError() << "\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }
        // ��Ƽĳ��Ʈ ���� �� TTL(Time-To-Live) �ɼ� ���� (�⺻ 1: ���� ��Ʈ��ũ)
        int ttl = 4;
        if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
            std::cerr << u8"setsockopt(IP_MULTICAST_TTL) failed\n";
            closesocket(sock_);
            WSACleanup();
            return false;
        }


        std::cout << u8"UDP Sender initialized for multicast group " << multicast_address
            << u8" on port " << port_ << "\n";
        return true;
    }

    // ��ü�� �´� ������ �����ϱ� ����
void UdpMulticast::setMissile(std::shared_ptr<Missile> m) {
        missile_ = m;
    }

MissilePacket UdpMulticast::serializeMissile(const Missile& missile) {
        MissilePacket packet;
        packet.EventCode = 3001;
        packet.BodyLength = 36;
        memset(packet.MissileId, 0, sizeof(packet.MissileId));
        strncpy_s(packet.MissileId, sizeof(packet.MissileId), missile.MissileId.c_str(), _TRUNCATE);
        packet.MissileState = missile.MissileState;
        packet.MissileLoc = missile.MissileLoc;
        return packet;
    }

    // ���� ����: �ֱ������� �����͸� ������ ����
void UdpMulticast::start() {
        running_ = true;
        sendThread_ = std::thread(&UdpMulticast::run, this);
    }

    // ���� ���� ����: 2.5�ʸ��� ������ ����
void UdpMulticast::run() {
        while (running_) {
            if (missile_) {
                MissilePacket packet = serializeMissile(*missile_);
                int sent = sendto(sock_, reinterpret_cast<const char*>(&packet), sizeof(packet), 0,
                    (sockaddr*)&destAddr_, sizeof(destAddr_));
                if (sent == SOCKET_ERROR) {
                    std::cerr << u8"sendto failed: " << WSAGetLastError() << "\n";
                }
                else if (missile_ -> MissileState == 2 || missile_->MissileState == 3 || missile_->MissileState == 4) {
                    std::cout << u8"Missile state is " << missile_->MissileState
                        << u8" �� stopping multicast.\n";

                    closesocket(sock_);
                    running_ = false;         
                    break;
                }
                else {
                    std::cout << u8" <Udp Multicast success> \n\n";
                    std::cout << u8"MSS-ID: " << missile_->MissileId << "\n"
                        << u8"Latitude: " << missile_->MissileLoc.latitude << "\n"
                        << u8"Longitude: " << missile_->MissileLoc.longitude << "\n"
                        << u8"Altitude: " << missile_->MissileLoc.altitude << "\n\n";
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }


    // ���� �� �ڿ� ����
void UdpMulticast::close() {
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

