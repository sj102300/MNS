#if 1
#pragma once
#include "UdpSender.hpp"
#include "UdpReceiver.hpp"
#include "UdpMuticast.hpp"
#include "MissileController.h"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include <atomic>
#include <vector>


// Packet�̶�� ������ �����ڿ��� 3���� receiver���� ���
//ThreadSafeQueue<Packet> sendQueue;

Location loc = { 101.0,201.0,10 };
int main() {
    //std::vector<std::shared_ptr<Missile>> missiles;

    //for (int i = 0;i < 3;++i) {
    //    auto missile = std::make_shared<Missile>();
    //    auto sender = std::make_shared<UdpSender1>();
    //    sender->init("239.0.0.1", 9876);
    //    auto controller = std::make_shared<MissileController>();

    //    missile->init(sender, controller);

    //    // 4. �̻��� �߻� ���� (��: �ӵ� 5.0f)
    //    missile->start(5.0f);

    //    // 5. ���Ϳ� ����
    //    missiles.push_back(missile);
    //}

    UdpSender1 sender;
    auto myMissile = std::make_shared<Missile>();
    MissileController controller;

    //std::string receiverAddress = "127.0.0.1"; // ���� IP
    std::string senderAddress = "239.0.0.1"; // ���� IP
    int senderPort = 10001;
    if (!sender.init(senderAddress, senderPort)) {
        std::cerr << "Sender initialization failed.\n";
        return -1;
    }


    sender.setMissile(myMissile);
    controller.setMissile(myMissile);
    controller.setTarget(loc); //loc�� ���� targetpoint ������!!!

    sender.start();
    controller.start(5.0);


    while (true) {
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "STOP") {
            sender.close();

           // receiver.close();
            controller.stop();
            break;
        }
    }
    // ���α׷� ���� ���� �����带 �����ؾ� �� ���
    std::cout << "Receiver and Sender threads completed.\n";

    return 0;
}
#endif
