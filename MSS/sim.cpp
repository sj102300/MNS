#if 1
#pragma once
#include "UdpSender.hpp"
#include "UdpReceiver.hpp"
#include "UdpMuticast.hpp"
#include "MissileController.h"

#include <thread>
#include <vector>

Location loc = { 101.0,201.0,10 };
int main() {

    std::vector<std::shared_ptr<Missile>> missiles;

    for (int i = 0;i < 1;++i) {
        auto missile = std::make_shared<Missile>();
        auto sender = std::make_shared<UdpMulticast>();
        sender->init("239.0.0.1", 9876);
        auto controller = std::make_shared<MissileController>();

        missile->init(sender, controller);

        // 4. �̻��� �߻� ���� (��: �ӵ� 5.0f)
        missile->start(5.0f);

        // 5. ���Ϳ� ����
        missiles.push_back(missile);
    }


    while (true) {
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "STOP") {
            break;
        }
    }
    // ���α׷� ���� ���� �����带 �����ؾ� �� ���
    std::cout << "Receiver and Sender threads completed.\n";

    return 0;
}
#endif
