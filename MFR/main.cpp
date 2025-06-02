#include "ScenarioManager.h"
#include "MFR.h"
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#pragma comment(lib, "ScenarioManager.lib")

using namespace sm;

// === ���� �� ===
const std::string SUBSYSTEM_ID = "MFR";
const std::string SCN_LOCAL_IP = "http://192.168.2.33:8080";  // ���� �ּ�
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080";  // SCN ���� �ּ�
const std::string UDP_IP = "239.0.0.1";        // ��Ƽĳ���� IP
const int         UDP_PORT = 9000;             // ��Ƽĳ���� ��Ʈ

// === �۷ι� ����ȭ ���� ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;

//running -> �ùķ��̼� ��
//         -> quit
//running -> �ùķ��̼� ���
//          -> start

std::shared_ptr<mfr::MFR> mfrPtr;           // MFR �ν��Ͻ��� �������� ���� �����ϰ�
std::thread radarThread;                    // Ž���� ������

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

    while (true) {
        // === "start" ��ȣ ���� ������ ��� ===
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }
        
        if (running) {
            // === ���� MFR ���� ó�� ===
            if (mfrPtr) {
                mfrPtr->stop();
                if (radarThread.joinable()) radarThread.join();
                mfrPtr.reset();
            }

            // === �ó����� ���� ��� ===
            Coordinate battery = scenarioRunner.getBatteryLocation();

            std::cout << u8"[" << SUBSYSTEM_ID << u8"] Ž�� ���� - ���� ��ġ: "
                << battery.latitude << ", " << battery.longitude << "\n";

            // === �� MFR �ν��Ͻ� ���� �� Ž�� ������ ���� ===
            mfrPtr = std::make_shared<mfr::MFR>(UDP_IP, UDP_PORT);

            radarThread = std::thread([battery]() {
                try {
                    mfrPtr->run(battery.latitude, battery.longitude, battery.altitude);
                }
                catch (const std::exception& e) {
                    std::cerr << "[" << SUBSYSTEM_ID << u8"] Ž�� ����: " << e.what() << "\n";
                }
            });
        }
        else {
            if (mfrPtr) {
                mfrPtr->stop();  // Ž�� ���� ���� ��û
            }

            if (radarThread.joinable()) {
                radarThread.join();  // Ž�� ������ ������� ���
            }
        }
    }

    // ���� ó��
    if (scenarioThread.joinable()) scenarioThread.join();
    if (radarThread.joinable()) radarThread.join();

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���α׷� ���� ����\n";
    return 0;
}