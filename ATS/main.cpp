#include "ATS.h"
#include "ScenarioManager.h"
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#pragma comment(lib, "ScenarioManager.lib")

using namespace sm;

// === 설정 값 ===
const std::string SUBSYSTEM_ID = "ATS";
const std::string SCN_LOCAL_IP = "http://192.168.2.55:8080/";  // 수신 주소
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080/";  // SCN 서버 주소

// === 글로벌 동기화 상태 ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;  // 변경 여부 플래그

// === 콜백 함수 정의 ===
void handleStart() {
    std::lock_guard<std::mutex> lock(mtx);

    if (running) {
        std::cout << u8"[" << SUBSYSTEM_ID << u8"] 이미 실행 중 - start 무시\n";
        return;
    }

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 시작 신호 수신\n";
    running = true;
    isChanged = true;
    cv.notify_one();  // 메인 스레드 깨우기
}

void handleQuit() {
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 종료 신호 수신\n";
    // ATS는 별도 종료 대상 없음 + Aircraft.exe는 개별 콘솔 실행이므로
    running = false;
    isChanged = true;
    cv.notify_one();
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 탐지 종료 완료, 다음 시작 신호 대기 중...\n";
}

// === main 함수 정의 ===
int main() {
    ScenarioManager scenarioRunner(
        SCN_LOCAL_IP,     // 수신 주소
        SCN_SERVER_IP,    // SCN 서버 주소
        SUBSYSTEM_ID         // 클라이언트 ID
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
            cv.wait(lock, [] { return isChanged.load(); });  // 변경점 감지
            isChanged = false;
        }

        if (running) {
            std::vector<AircraftInfo> aircraftList = scenarioRunner.getAircraftList();
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] 총 " << aircraftList.size() << u8"대 항공기 실행 시작\n";

            ats.setAircraftList(aircraftList);
            ats.launchAll();

            std::cout << u8"[" << SUBSYSTEM_ID << u8"] 항공기 실행 완료, 다음 명령 대기 중...\n";
        }
        else {
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] 시나리오 종료일 때\n";
            ats.terminateAll();
        }
    }


    if (scenarioThread.joinable()) scenarioThread.join();

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 프로그램 정상 종료\n";
    return 0;
}
