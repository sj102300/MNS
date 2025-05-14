#if 1
#pragma once
#include "UdpSender.hpp"
#include "UdpReceiver.hpp"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include <atomic>

std::atomic<bool> running(true);
Location impact_point;
uint32_t mss_state = 0;
// Packet이라는 구조의 공유자원을 3개의 receiver들이 사용
//ThreadSafeQueue<Packet> sendQueue;

int main() {
    Scenario scenario = parseScenarioFromFile("example.json");

    UdpReceiver receiver;
    UdpSender sender;

    // 수신 객체 초기화
    std::string receiverAddress = "127.0.0.1"; // 예시 IP
    int receiverPort = 10001;
    if (!receiver.init(receiverAddress, receiverPort)) {
        std::cerr << "Receiver initialization failed.\n";
        return -1;
    }

    // 송신 객체 초기화
    std::string senderAddress = "127.0.0.1"; // 예시 IP
    int senderPort = 20000;
    if (!sender.init(senderAddress, senderPort)) {
        std::cerr << "Sender initialization failed.\n";
        return -1;
    }
    // 객체에 packet 설정해서 packet 업데이트 필요
    receiver.start();
    sender.start();

    while (true) {
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "STOP") {
            sender.close();
            receiver.close();
            break;
        }
    }
    // 프로그램 종료 전에 쓰레드를 종료해야 할 경우
    std::cout << "Receiver and Sender threads completed.\n";

    return 0;
}
#endif


#if 0
#pragma once
// main.cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#include "UdpReceiver.hpp"
#include "UdpSender.hpp"

std::atomic<bool> running(true);

void recvThread(UdpReceiver& receiver) {
    receiver.start();  // 내부에서 receive() 스레드 분기함
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    receiver.close();  // running false되면 소켓 닫기
}

void sendThread(UdpSender& sender) {
    sender.start(); // 내부에서 run() 스레드 분기함
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    sender.close();
}

int main() {
    // 수신기 초기화
    UdpReceiver receiver;
    if (!receiver.init("0.0.0.0", 9000)) {
        std::cerr << "Receiver 초기화 실패\n";
        return -1;
    }

    // 송신기 초기화 (자기 자신에게 보내도록 설정: loopback)
    UdpSender sender;
    if (!sender.init("127.0.0.1", 9000)) {
        std::cerr << "Sender 초기화 실패\n";
        return -1;
    }

    // 쓰레드 시작
    std::thread t1(recvThread, std::ref(receiver));
    std::thread t2(sendThread, std::ref(sender));

    // 10초간 실행 후 종료
    std::this_thread::sleep_for(std::chrono::seconds(10));
    running = false;

    // 쓰레드 정리
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();

    std::cout << "시뮬레이터 종료\n";
    return 0;
}
#endif