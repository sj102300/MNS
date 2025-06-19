#if 0
#pragma once
#include "LchHandler.h"
#include <iostream>

int main() {
    // ���� ��Ƽĳ��Ʈ �ּ� �� ��Ʈ (TCC -> LCH)
    std::string recvMulticastAddr = "239.0.0.1";  // ���� ��Ƽĳ��Ʈ �ּ�
    int recvPort = 9000;

    // �۽� ��Ƽĳ��Ʈ �ּ� �� ��Ʈ (LCH -> MSS)
    std::string sendMulticastAddr = "239.0.0.1";  // ���� ��Ƽĳ��Ʈ �ּ�
    int sendPort = 9000;

    LCHLauncher launcher;

    if (!launcher.init(recvMulticastAddr, recvPort, sendMulticastAddr, sendPort)) {
        std::cerr << "LCHLauncher �ʱ�ȭ ����\n";
        return 1;
    }

    std::cout << "LCH �ý��� ����...\n";

    launcher.run();  // ���� ���� ����

    return 0;
}
#endif

#if 1
#pragma once
#include "LchHandler.h"
#include "ScenarioManager.h"
#include <iostream>
#include <cstring> 
#include <vector>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <thread>
#pragma comment(lib, "ScenarioManager.lib")

using namespace sm;

// === ���� �� ===
const std::string SUBSYSTEM_ID = "LCH";
const std::string SCN_LOCAL_IP = "http://192.168.2.11:8080";  // ���� �ּ�
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080";  // SCN ���� �ּ�
const std::string UDP_IP = "239.0.0.1";        // ��Ƽĳ���� IP
const int         UDP_PORT = 9000;             // ��Ƽĳ���� ��Ʈ

// === �۷ι� ����ȭ ���� ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;

std::thread launcherThread;
LCHLauncher launcher;

//running -> �ùķ��̼� ��
//         -> quit
//running -> �ùķ��̼� ���
//          -> start


// === �ݹ� �Լ� ���� ===
void handleStart() {
    std::lock_guard<std::mutex> lock(mtx);

    if (running) {
        std::cout << u8"[" << SUBSYSTEM_ID << u8"] �̹� ���� �� - start ����\n";
        return;
    }

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���� ��ȣ ����\n";
    running = true;
    isChanged = true;
    cv.notify_one();  // ���� ������ �����
}

void handleQuit() {         //MFR->SM����
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���� ��ȣ ����\n";

    if (!running) {
        std::cout << u8"[" << SUBSYSTEM_ID << u8"] �ó����� ��� �� - quit ����\n";
        return;
    }

    running = false;
    isChanged = true;
    cv.notify_one();
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] Ž�� ���� �Ϸ�, ���� ���� ��ȣ ��� ��...\n";
}

// === main �Լ� ���� ===
int main() {
    ScenarioManager scenarioRunner(
        SCN_LOCAL_IP,     // ���� �ּ�
        SCN_SERVER_IP,    // SCN ���� �ּ�
        SUBSYSTEM_ID         // Ŭ���̾�Ʈ ID
    );
    
    scenarioRunner.setOnReadyCallback(handleStart);
    scenarioRunner.setOnQuitCallback(handleQuit);
    
    std::thread scenarioThread([&]() {
        scenarioRunner.run();  // blocking
        });
    std::cout << u8"�ó����� ��ȣ �ޱ���..?" << "\n";
    while (true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }
        if (running) {
            std::cout << "[" << SUBSYSTEM_ID << "] LCH �ý��� ����...\n";

            if (!launcher.init(UDP_IP, 9000,UDP_IP,9000)) {
                std::cerr << "[" << SUBSYSTEM_ID << "] �ʱ�ȭ ����! ������ ���� ����\n";
                continue;
            }
            
            if (launcherThread.joinable()) {
                launcherThread.join(); // ���� thread�� ���� �־��ٸ� �ݵ�� ����
            }

            launcherThread = std::thread([&]() {
                launcher.run();
                });
        }
        else {
            std::cout << "[" << SUBSYSTEM_ID << "] �ùķ��̼� ���� ó�� ��...\n";

            launcher.stop();  // run ���� ���� Ż�� ����

            if (launcherThread.joinable()) {
                launcherThread.join();
            }

            std::cout << "[" << SUBSYSTEM_ID << "] LCH �ý��� ���� �Ϸ�, ��� ���� ����\n";
        }
    }

    if (scenarioThread.joinable()) scenarioThread.join();

    std::cout << "[" << SUBSYSTEM_ID << "] ���α׷� ����\n";
    return 0;
}
#endif
