#include "ScenarioManager.h"
#include <condition_variable>
#include <mutex>
#include <iostream>

using namespace sm;

// === �۷ι� ����ȭ ���� ===
std::mutex mtx;
std::condition_variable cv;
bool started = false;  // ���� ������ �ƴ϶� '���۵�' �˸��� �÷���

// === �ݹ� �Լ� ���� ===
void handleStart() { 
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << u8"[MFR] ���� ��ȣ ����\n";
    started = true;
    cv.notify_one();  // ���� ������ �����
}

void handleQuit() {
    std::cout << u8"[MFR] ���� ��ȣ ����\n";
}

// === main �Լ� ���� ===
int main() {
    ScenarioManager scenarioRunner(
        "http://192.168.2.31:8080",   // ���� �ּ�
        "http://192.168.2.30:8080",  // SCN ���� �ּ�
        "MFR"                         // Ŭ���̾�Ʈ ID
    );

    scenarioRunner.setOnReadyCallback(handleStart);
    scenarioRunner.setOnQuitCallback(handleQuit);

    scenarioRunner.run();  // ������ ���� ���� (blocking)

    // �ݺ������� start ��ȣ ���
    while (true) {

        // ���� ������� "���۵�" �˸� �� ������ ���
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return started; });
            started = false;  // ���� ��⸦ ���� �ʱ�ȭ
        }

        // === �ó����� ���� ��� ===
        Coordinate battery = scenarioRunner.getBatteryLocation();
        std::vector<AircraftInfo> aircrafts = scenarioRunner.getAircraftList();
    }

    return 0;
}