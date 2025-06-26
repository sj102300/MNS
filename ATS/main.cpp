#include "ATS.h"
#include "ScenarioManager.h"
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#pragma comment(lib, "ScenarioManager.lib")

using namespace sm;

// === ���� �� ===
const std::string SUBSYSTEM_ID = "ATS";
const std::string SCN_LOCAL_IP = "http://192.168.2.55:8080/";  // ���� �ּ�
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080/";  // SCN ���� �ּ�

// === �۷ι� ����ȭ ���� ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;  // ���� ���� �÷���

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

void handleQuit() {
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���� ��ȣ ����\n";
    // ATS�� ���� ���� ��� ���� + Aircraft.exe�� ���� �ܼ� �����̹Ƿ�
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

    ATS ats;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });  // ������ ����
            isChanged = false;
        }

        if (running) {
            std::vector<AircraftInfo> aircraftList = scenarioRunner.getAircraftList();
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] �� " << aircraftList.size() << u8"�� �װ��� ���� ����\n";

            ats.setAircraftList(aircraftList);
            ats.launchAll();

            std::cout << u8"[" << SUBSYSTEM_ID << u8"] �װ��� ���� �Ϸ�, ���� ��� ��� ��...\n";
        }
        else {
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] �ó����� ������ ��\n";
            ats.terminateAll();
        }
    }


    if (scenarioThread.joinable()) scenarioThread.join();

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���α׷� ���� ����\n";
    return 0;
}
