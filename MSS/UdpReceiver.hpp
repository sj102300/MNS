#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Ireceiver.hpp"
#include "UdpSender.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

class UdpReceiver : public IReceiver {
public:
    UdpReceiver() : sock_(INVALID_SOCKET), port_(0), running_(false) {}
    ~UdpReceiver() override { close(); }

    void setSender(UdpSender* sender) {
        sender_ = sender;
    }
    bool init(const std::string& address, int port) override {
        port_ = port;

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

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed\n";
            closesocket(sock_);
            return false;
        }

        std::cout << "UDP Receiver initialized on port " << port_ << "\n";
        return true;
    }
  
    void start() override {
        running_ = true;
        std::thread(&UdpReceiver::receive, this).detach();
    }

    void receive() override {
        char buffer[1024];
        sockaddr_in senderAddr{};
        int senderAddrLen = sizeof(senderAddr);
        while (running_) {
            int recvLen = recvfrom(sock_, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrLen);
            if (recvLen == SOCKET_ERROR) {
                std::cerr << "recvfrom failed\n";
                continue;
            }
            handleData(buffer, recvLen);
        }
    }
    // Port: 10001(�߻� ���), 10002(�߻� ����), 10003(���� ����), 10004(����), 10005(��� ����)
    void handleData(const char* buffer, int length) override {
        
        Packet pkg = {};
        if (port_ == 10001) { // �߻����� ������ -> impact point �� �����ϰ� ����� �̵��ϴ� ������ �ʿ� + ���� send�� 0.01�ʸ��� �ѹ��� ���־����
            const LaunchPacket* pkt1 = reinterpret_cast<const LaunchPacket*>(buffer);
            impact_point = pkt1->ImpactPoint;
            pkg.MissileState = 2; // �߻� ����� ������ �߻���·� ��������!!
            strcpy(pkg.MissileId,pkt1->MissileId);
            pkg.MissileLoc.altitude = 0; // �⺻ ��ġ
            pkg.MissileLoc.latitude = 0;
            pkg.MissileLoc.longitude = 0;

            if (sender_) sender_->setPacket(pkg);
            // �̻��� ��ġ��?? ���⼭ ������ ���� �ʿ���� �׳� default ���� �ؾ���
        }
        else if (port_ == 10003) { // ���� ������ ������ ���ϰ� �������� �����带 �� �ݰ� �̻����� �����ؾ���
            const InterceptResultPacket* pkt2 = reinterpret_cast<const InterceptResultPacket*>(buffer);
            running_ = false; // �ݺ��Ǵ� ������ ���� �ʿ�
            close();
        }
        else if (port_ == 10005) { // ������� ����� ������ ���� �������� ������� ������ �ݰ� �̻��� ����
            const EDPacket* pkt3 = reinterpret_cast<const EDPacket*>(buffer);
        }else{
            cout << " �߸��� ��Ʈ��ȣ �����Դϴ� !!! \n";
        }


        //const LaunchPacket* pkt = reinterpret_cast<const LaunchPacket*>(buffer);
        //std::cout << "LaunchPacket ����: " << pkt->MissileId << std::endl;
            
    }
    void close() override {

        running_ = false;
        if (sock_ != INVALID_SOCKET) {
            closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
        WSACleanup();
    }

private:
    SOCKET sock_;
    int port_;
    bool running_;
    thread recv_thread;
    Location impact_point;
    UdpSender* sender_ = nullptr;
};