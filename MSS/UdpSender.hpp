#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "Isender.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

class UdpSender : public Isender {
public:
    UdpSender() : sock_(INVALID_SOCKET), running_(false) {}
    ~UdpSender() override { close(); }

    bool init(const std::string& address, int port) override {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return false;
        }

        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET) {
            std::cerr << "Failed to create socket\n";
            return false;
        }

        memset(&remoteAddr_, 0, sizeof(remoteAddr_));
        remoteAddr_.sin_family = AF_INET;
        remoteAddr_.sin_port = htons(port);
        inet_pton(AF_INET, address.c_str(), &remoteAddr_.sin_addr);

        std::cout << "UDP Sender initialized to " << address << ":" << port << "\n";
        return true;
    }

    void setPacket(const Missile& pkt) {
        std::lock_guard<std::mutex> lock(packetMutex_);
        currentPacket_ = pkt;
    }
    
    void start() {
        running_ = true;
        senderThread_ = std::thread(&UdpSender::run, this);
    }
    void run() override {
        while (running_) {
            Missile pktCopy;
            {
                std::lock_guard<std::mutex> lock(packetMutex_);
                pktCopy = currentPacket_;
            }

            send(reinterpret_cast<const char*>(&pktCopy), sizeof(Missile));

            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 0.01초 주기
        }
    }

    bool send(const char* data, int length) override {
        cout << "data 길이는 " << length << "\n";
        int sent = sendto(sock_, data, length, 0, (sockaddr*)&remoteAddr_, sizeof(remoteAddr_));
        if (sent == SOCKET_ERROR) {
            std::cerr << "sendto failed: " << WSAGetLastError() << "\n";
            return false;
        }
        return true;
    }



    void close() override {
        running_ = false;
        if (senderThread_.joinable()) {
            senderThread_.join();  // 스레드 종료 대기
        }
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
        WSACleanup();
    }

private:
    SOCKET sock_;
    sockaddr_in remoteAddr_;
    std::thread senderThread_;
    std::atomic<bool> running_;


    Missile currentPacket_;
    std::mutex packetMutex_;
    std::atomic<bool> packetReady_ = false;
};


//InterceptResultPacket pkt{};
//cout << "packet size: " << sizeof(pkt);
//pkt.EvenetCode = 1001;
//strcpy(pkt.EventId, "EDYYYYMMDHHMMSSmm2");
//strcpy(pkt.MissileId, "MSL-200");
//strcpy(pkt.AirCraftId, "ACF-277");
////pkt.ImpactPoint = { 2,3,4 };
//send(reinterpret_cast<const char*>(&pkt), sizeof(pkt));
//std::this_thread::sleep_for(std::chrono::microseconds(1000));