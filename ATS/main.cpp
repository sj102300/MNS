#include "ATS.h"
#include "ScenarioManager.h"
#include "AircraftWorker.h"  // ats::AircraftInfo
#include "ShootDownThread.h"
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#include <windows.h>

#pragma comment(lib, "ScenarioManager.lib")

using namespace sm;

// === 설정 값 ===
const std::string SUBSYSTEM_ID = "ATS";
const std::string SCN_LOCAL_IP = "http://192.168.2.55:8080/";
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080/";

// === 글로벌 동기화 상태 ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;

// === 콜백 함수 정의 ===
void handleStart() {
    std::lock_guard<std::mutex> lock(mtx);
    if (running) {
        //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 이미 실행 중 - start 무시\n";
        return;
    }

    //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 시작 신호 수신\n";
    running = true;
    isChanged = true;
    cv.notify_one();
}

void handleQuit() {
    //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 종료 신호 수신\n";
    running = false;
    isChanged = true;
    cv.notify_one();
    //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 모든 항공기 종료 요청 완료\n";
}

// === main 함수 정의 ===
int main() {
    SetConsoleOutputCP(CP_UTF8);
    initializeMultiSenderSocket();

    ScenarioManager scenarioRunner(
        SCN_LOCAL_IP,
        SCN_SERVER_IP,
        SUBSYSTEM_ID
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
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }

        if (running) {
            std::vector<sm::AircraftInfo> rawList = scenarioRunner.getAircraftList();
            std::vector<ats::AircraftInfo> converted;

            for (const auto& a : rawList) {
                ats::AircraftInfo temp;
                temp.id = a.aircraft_id;
                temp.startPoint = { a.start_point.latitude, a.start_point.longitude };
                temp.finishPoint = { a.end_point.latitude, a.end_point.longitude };
                temp.currentPoint = temp.startPoint;
                temp.IFF = !a.friend_or_foe.empty() ? a.friend_or_foe.at(0) : 'U';  // 'U' = Unknown fallback

                converted.push_back(temp);
            }

            //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 총 " << converted.size() << u8"대 항공기 스레드 실행 시작\n";
            ats.setAircraftList(converted);
            ats.launchAll();
            //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 항공기 실행 완료, 다음 명령 대기 중...\n";
        }
        else {
            //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 시나리오 종료, 스레드 종료 시작\n";
            ats.terminateAll();
        }
    }

    if (scenarioThread.joinable()) scenarioThread.join();

    //std::cout << u8"[" << SUBSYSTEM_ID << u8"] 프로그램 정상 종료\n";
    return 0;
}
