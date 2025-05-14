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
// Packet�̶�� ������ �����ڿ��� 3���� receiver���� ���
//ThreadSafeQueue<Packet> sendQueue;

int main() {
    Scenario scenario = parseScenarioFromFile("example.json");

    UdpReceiver receiver;
    UdpSender sender;

    // ���� ��ü �ʱ�ȭ
    std::string receiverAddress = "127.0.0.1"; // ���� IP
    int receiverPort = 10001;
    if (!receiver.init(receiverAddress, receiverPort)) {
        std::cerr << "Receiver initialization failed.\n";
        return -1;
    }

    // �۽� ��ü �ʱ�ȭ
    std::string senderAddress = "127.0.0.1"; // ���� IP
    int senderPort = 20000;
    if (!sender.init(senderAddress, senderPort)) {
        std::cerr << "Sender initialization failed.\n";
        return -1;
    }
    // ��ü�� packet �����ؼ� packet ������Ʈ �ʿ�
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
    // ���α׷� ���� ���� �����带 �����ؾ� �� ���
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
    receiver.start();  // ���ο��� receive() ������ �б���
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    receiver.close();  // running false�Ǹ� ���� �ݱ�
}

void sendThread(UdpSender& sender) {
    sender.start(); // ���ο��� run() ������ �б���
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    sender.close();
}

int main() {
    // ���ű� �ʱ�ȭ
    UdpReceiver receiver;
    if (!receiver.init("0.0.0.0", 9000)) {
        std::cerr << "Receiver �ʱ�ȭ ����\n";
        return -1;
    }

    // �۽ű� �ʱ�ȭ (�ڱ� �ڽſ��� �������� ����: loopback)
    UdpSender sender;
    if (!sender.init("127.0.0.1", 9000)) {
        std::cerr << "Sender �ʱ�ȭ ����\n";
        return -1;
    }

    // ������ ����
    std::thread t1(recvThread, std::ref(receiver));
    std::thread t2(sendThread, std::ref(sender));

    // 10�ʰ� ���� �� ����
    std::this_thread::sleep_for(std::chrono::seconds(10));
    running = false;

    // ������ ����
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();

    std::cout << "�ùķ����� ����\n";
    return 0;
}
#endif