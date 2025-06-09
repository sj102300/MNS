#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Missile.h"
#include "Isender.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>

#pragma comment(lib, "ws2_32.lib")


class UdpMulticast : public Isender {

public:
     UdpMulticast();

        // ��Ƽĳ��Ʈ ��� �ּҿ� ��Ʈ�� �޾� �ʱ�ȭ
     virtual bool init(const std::string& multicast_address, int port);
        // ��ü�� �´� ������ �����ϱ� ����
     virtual void setMissile(std::shared_ptr<Missile> m);
        // ���� ����: �ֱ������� �����͸� ������ ����
    virtual void start();
        // ���� ���� ����: 2.5�ʸ��� ������ ����
    virtual void run();
        // ���� �� �ڿ� ����
    virtual void close();

    MissilePacket serializeMissile(const Missile& missile);

private:
    SOCKET sock_;
    sockaddr_in destAddr_;  // ��Ƽĳ��Ʈ ��� �ּ� ����
    int port_;
    bool running_;
    std::thread sendThread_;
    std::shared_ptr<Missile> missile_;
};
