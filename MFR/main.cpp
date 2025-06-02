#include "ScenarioManager.h"
#include "MFR.h"
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#pragma comment(lib, "ScenarioManager.lib")

using namespace sm;

// === 설정 값 ===
const std::string SUBSYSTEM_ID = "MFR";
const std::string SCN_LOCAL_IP = "http://192.168.2.33:8080";  // 수신 주소
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080";  // SCN 서버 주소
const std::string UDP_IP = "239.0.0.1";        // 멀티캐스팅 IP
const int         UDP_PORT = 9000;             // 멀티캐스팅 포트

// === 글로벌 동기화 상태 ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;

//running -> 시뮬레이션 중
//         -> quit
//running -> 시뮬레이션 대기
//          -> start

std::shared_ptr<mfr::MFR> mfrPtr;           // MFR 인스턴스를 전역에서 접근 가능하게
std::thread radarThread;                    // 탐지용 스레드

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

void handleQuit() {         //MFR->SM으로
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 종료 신호 수신\n";

    if (!running) {
        std::cout << u8"[" << SUBSYSTEM_ID << u8"] 시나리오 대기 중 - quit 무시\n";
        return;
    }

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

    while (true) {
        // === "start" 신호 오기 전까지 대기 ===
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }
        
        if (running) {
            // === 이전 MFR 종료 처리 ===
            if (mfrPtr) {
                mfrPtr->stop();
                if (radarThread.joinable()) radarThread.join();
                mfrPtr.reset();
            }

            // === 시나리오 정보 출력 ===
            Coordinate battery = scenarioRunner.getBatteryLocation();

            std::cout << u8"[" << SUBSYSTEM_ID << u8"] 탐지 시작 - 포대 위치: "
                << battery.latitude << ", " << battery.longitude << "\n";

            // === 새 MFR 인스턴스 생성 및 탐지 스레드 시작 ===
            mfrPtr = std::make_shared<mfr::MFR>(UDP_IP, UDP_PORT);

            radarThread = std::thread([battery]() {
                try {
                    mfrPtr->run(battery.latitude, battery.longitude, battery.altitude);
                }
                catch (const std::exception& e) {
                    std::cerr << "[" << SUBSYSTEM_ID << u8"] 탐지 오류: " << e.what() << "\n";
                }
            });
        }
        else {
            if (mfrPtr) {
                mfrPtr->stop();  // 탐지 루프 종료 요청
            }

            if (radarThread.joinable()) {
                radarThread.join();  // 탐지 스레드 종료까지 대기
            }
        }
    }

    // 종료 처리
    if (scenarioThread.joinable()) scenarioThread.join();
    if (radarThread.joinable()) radarThread.join();

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 프로그램 정상 종료\n";
    return 0;
}