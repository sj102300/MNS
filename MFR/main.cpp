#include "ScenarioInit.h"
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

void handleQuit(ScenarioInit& runner) {
    std::cout << u8"[MFR] ���� ��ȣ ����\n";
    runner.handleQuitSignal();  // ���ο��� ���� �ʱ�ȭ �� reset ó��
}

// === main �Լ� ���� ===
int main() {
    //ScenarioInit scenarioRunner(
    //    "http://192.168.15.3:8080",   // ���� �ּ�
    //    "http://192.168.15.30:8080",  // SCN ���� �ּ�
    //    "MFR"                         // Ŭ���̾�Ʈ ID
    //);
    ScenarioInit scenarioRunner(
        "http://localhost:8080",   // ���� �ּ�
        "http://localhost:8000",  // SCN ���� �ּ�
        "MFR"                         // Ŭ���̾�Ʈ ID
    );

    scenarioRunner.setOnReadyCallback(handleStart);
    scenarioRunner.setOnQuitCallback(nullptr);

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