#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <gtest/gtest.h>
#include "IReceiver.h" // �� �κ� �׽��� ���� �߰�
#include "Missile.h"
#include "Aircraft.h"

class UdpReceiver : public IReceiver {
public:
    UdpReceiver();

    // ��Ƽĳ��Ʈ �ּҿ� ��Ʈ�� ������ �ʱ�ȭ
    virtual bool init(const std::string& multicast_address, int port);

    // ���� ���� ����
    virtual void start();

    // ���� ���� ����
    virtual void run();

    // ���� �� �ڿ� ����
    virtual void close();

    // �̻��� ����
    void setMissileMap(const std::unordered_map<std::string, std::shared_ptr<Missile>>& map); // 6���� �̻��� ��ü�� �����ϱ� ����


private:
    SOCKET sock_;
    sockaddr_in localAddr_;
    ip_mreq mreq_;  // ��Ƽĳ��Ʈ �׷� ����
    std::atomic<bool> running_;
    std::thread recvThread_;
    int port_;
    std::shared_ptr<Missile> missile_;
    std::unordered_map<std::string, std::shared_ptr<Missile>> missile_map_;
    std::shared_ptr<Aircraft> aircraft_;
    std::unordered_map <std::string, std::shared_ptr<Aircraft>> Aircraft_map_;
};
