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


// Packet�̶�� ������ �����ڿ��� 3���� receiver���� ���
//ThreadSafeQueue<Packet> sendQueue;

Location loc = { 101.0,201.0,10 };

int main() {

  //  UdpReceiver receiver;
    UdpSender1 sender;
    auto myMissile = std::make_shared<Missile>(Missile{ "M-0001", 1, {0.0, 0.0, 0.0} });
    MissileController controller;

    // ���� ��ü �ʱ�ȭ
    std::string receiverAddress = "127.0.0.1"; // ���� IP
    //int receiverPort = 10001;
    //if (!receiver.init(receiverAddress, receiverPort)) {
    //    std::cerr << "Receiver initialization failed.\n";
    //    return -1;
    //}

    // �۽� ��ü �ʱ�ȭ
    std::string senderAddress = "239.0.0.1"; // ���� IP
    int senderPort = 10001;
    if (!sender.init(senderAddress, senderPort)) {
        std::cerr << "Sender initialization failed.\n";
        return -1;
    }

    sender.setMissile(myMissile);
    controller.setMissile(myMissile);
    controller.setTarget(loc); //loc�� ���� targetpoint ������!!!

    // ��ü�� packet �����ؼ� packet ������Ʈ �ʿ�
   // receiver.start();
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
