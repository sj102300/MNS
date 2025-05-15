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


// Packet이라는 구조의 공유자원을 3개의 receiver들이 사용
//ThreadSafeQueue<Packet> sendQueue;

Location loc = { 101.0,201.0,10 };

int main() {

  //  UdpReceiver receiver;
    UdpSender1 sender;
    auto myMissile = std::make_shared<Missile>(Missile{ "M-0001", 1, {0.0, 0.0, 0.0} });
    MissileController controller;

    // 수신 객체 초기화
    std::string receiverAddress = "127.0.0.1"; // 예시 IP
    //int receiverPort = 10001;
    //if (!receiver.init(receiverAddress, receiverPort)) {
    //    std::cerr << "Receiver initialization failed.\n";
    //    return -1;
    //}

    // 송신 객체 초기화
    std::string senderAddress = "239.0.0.1"; // 예시 IP
    int senderPort = 10001;
    if (!sender.init(senderAddress, senderPort)) {
        std::cerr << "Sender initialization failed.\n";
        return -1;
    }

    sender.setMissile(myMissile);
    controller.setMissile(myMissile);
    controller.setTarget(loc); //loc로 임의 targetpoint 설정함!!!

    // 객체에 packet 설정해서 packet 업데이트 필요
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
    // 프로그램 종료 전에 쓰레드를 종료해야 할 경우
    std::cout << "Receiver and Sender threads completed.\n";

    return 0;
}
#endif
