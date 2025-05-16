#include "ScenarioInit.h"
#include <condition_variable>
#include <mutex>
#include <iostream>

using namespace sm;

// === �۷ι� ����ȭ ���� ===
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

// === �ݹ� �Լ� ���� ===
void handleStart() {
    std::lock_guard<std::mutex> lock(mtx);
    ready = true;
    cv.notify_one();
}

void handleQuit(ScenarioInit& runner) {
    runner.handleQuitSignal();
}

// === main �Լ� ���� ===
int main() {
    ScenarioInit scenarioRunner(
        "http://127.0.0.1:9015",  // �� �ó����� ���� �ּ�
        "http://127.0.0.1:8000",  // ��û�� SCN ���� �ּ�
        "MFR"                     // �� Ŭ���̾�Ʈ ID
    );
    
    scenarioRunner.setOnReadyCallback(handleStart);

    scenarioRunner.setOnQuitCallback([&]() {
        handleQuit(scenarioRunner);
        });

    scenarioRunner.run();

    // ���� �����忡�� ���
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });

    //ScenarioInfo info = scenarioRunner.getScenarioInfo();
    Coordinate battery = scenarioRunner.getBatteryLocation();
    //std::vector<AircraftInfo> aircrafts = scenarioRunner.getAircraftList();

    //////////////////////////////////  ��� Ȯ��  ///////////////////////////////////
    std::cout << "\n" << u8"===============================================" << "\n";
    //std::cout << u8"�ó����� ID��: " << info.scenario_id << std::endl;
    std::cout << u8"[���� ��ġ] ����: " << battery.latitude
        << u8", �浵: " << battery.longitude
        << u8", ��: " << battery.altitude << std::endl;
    //for (const auto& ac : aircrafts) {
    //    std::cout << u8"[�װ���] ID: " << ac.aircraft_id << u8", �Ǿ�: " << ac.friend_or_foe << std::endl;
    //}
    //////////////////////////////////////////////////////////////////////////////////




    std::cin.get(); // ���α׷� ���� �ӽ� ����
    return 0;
}
