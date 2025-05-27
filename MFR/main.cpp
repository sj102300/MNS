#include "ScenarioManager.h"
#include <condition_variable>
#include <mutex>
#include <iostream>

using namespace sm;

// === 글로벌 동기화 상태 ===
std::mutex mtx;
std::condition_variable cv;
bool started = false;  // 상태 관리가 아니라 '시작됨' 알림용 플래그

// === 콜백 함수 정의 ===
void handleStart() { 
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << u8"[MFR] 시작 신호 수신\n";
    started = true;
    cv.notify_one();  // 메인 스레드 깨우기
}

void handleQuit() {
    std::cout << u8"[MFR] 종료 신호 수신\n";
}

// === main 함수 정의 ===
int main() {
    ScenarioManager scenarioRunner(
        "http://192.168.2.31:8080",   // 수신 주소
        "http://192.168.2.30:8080",  // SCN 서버 주소
        "MFR"                         // 클라이언트 ID
    );

    scenarioRunner.setOnReadyCallback(handleStart);
    scenarioRunner.setOnQuitCallback(handleQuit);

    scenarioRunner.run();  // 리스닝 루프 진입 (blocking)

    // 반복적으로 start 신호 대기
    while (true) {

        // 메인 스레드는 "시작됨" 알림 올 때까지 대기
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return started; });
            started = false;  // 다음 대기를 위해 초기화
        }

        // === 시나리오 정보 출력 ===
        Coordinate battery = scenarioRunner.getBatteryLocation();
        std::vector<AircraftInfo> aircrafts = scenarioRunner.getAircraftList();
    }

    return 0;
}