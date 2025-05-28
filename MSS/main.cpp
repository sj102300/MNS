#if 1
#pragma once
#include "UdpReceiver.h"
#include "UdpMuticast.h"
#include "MissileController.h"

#include <thread>
#include <vector>

int main() {

    std::unordered_map<std::string, std::shared_ptr<Missile>> missileMap;
    std::vector<std::shared_ptr<Missile>> missiles;

    for (int i = 0; i < 6; ++i) {
        auto missile = std::make_shared<Missile>();

        // ���� ID �ο�
        std::string id = "MSS-" + std::to_string(100 + i);
        missile->setMissileId(id);  // �̸� ������ setter

        // Multicast, Controller ���� �� �ʱ�ȭ
        auto sender = std::make_shared<UdpMulticast>();
        sender->init("239.0.0.1", 9001);
        auto controller = std::make_shared<MissileController>();

        missile->init(sender, controller);
        missile->start(2000.0f);

        // map �� vector�� ����
        missileMap[id] = missile;
        missiles.push_back(missile);
    }

    auto receiver = std::make_shared<UdpReceiver>();
    receiver->init("239.0.0.1", 9001);  // ���� ��Ʈ �� ���ε� �ּ� ����
    receiver->setMissileMap(missileMap);
    receiver->start();  // ���� ������ ����

    std::cout << "�̻��� ���� ��� ��...\n";
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    // ���α׷� ���� ���� �����带 �����ؾ� �� ���
    std::cout << "Receiver and Sender threads completed.\n";

    return 0;
}
#endif
