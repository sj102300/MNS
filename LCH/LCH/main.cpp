#if 0
#pragma once
#include "LchHandler.h"
#include <iostream>

int main() {
    // 수신 멀티캐스트 주소 및 포트 (TCC -> LCH)
    std::string recvMulticastAddr = "239.0.0.1";  // 예시 멀티캐스트 주소
    int recvPort = 9000;

    // 송신 멀티캐스트 주소 및 포트 (LCH -> MSS)
    std::string sendMulticastAddr = "239.0.0.1";  // 예시 멀티캐스트 주소
    int sendPort = 9000;

    LCHLauncher launcher;

    if (!launcher.init(recvMulticastAddr, recvPort, sendMulticastAddr, sendPort)) {
        std::cerr << "LCHLauncher 초기화 실패\n";
        return 1;
    }

    std::cout << "LCH 시스템 시작...\n";

    launcher.run();  // 무한 루프 진입

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

// === 설정 값 ===
const std::string SUBSYSTEM_ID = "LCH";
const std::string SCN_LOCAL_IP = "http://192.168.2.11:8080";  // 수신 주소
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080";  // SCN 서버 주소
const std::string UDP_IP = "239.0.0.1";        // 멀티캐스팅 IP
const int         UDP_PORT = 9000;             // 멀티캐스팅 포트

// === 글로벌 동기화 상태 ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;

std::thread launcherThread;
LCHLauncher launcher;

//running -> 시뮬레이션 중
//         -> quit
//running -> 시뮬레이션 대기
//          -> start


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
    std::cout << u8"시나리오 신호 받기전..?" << "\n";
    while (true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }
        if (running) {
            std::cout << "[" << SUBSYSTEM_ID << "] LCH 시스템 실행...\n";

            if (!launcher.init(UDP_IP, 9000,UDP_IP,9000)) {
                std::cerr << "[" << SUBSYSTEM_ID << "] 초기화 실패! 스레드 실행 안함\n";
                continue;
            }
            
            if (launcherThread.joinable()) {
                launcherThread.join(); // 이전 thread가 돌고 있었다면 반드시 정리
            }

            launcherThread = std::thread([&]() {
                launcher.run();
                });
        }
        else {
            std::cout << "[" << SUBSYSTEM_ID << "] 시뮬레이션 종료 처리 중...\n";

            launcher.stop();  // run 루프 내부 탈출 유도

            if (launcherThread.joinable()) {
                launcherThread.join();
            }

            std::cout << "[" << SUBSYSTEM_ID << "] LCH 시스템 종료 완료, 대기 상태 진입\n";
        }
    }

    if (scenarioThread.joinable()) scenarioThread.join();

    std::cout << "[" << SUBSYSTEM_ID << "] 프로그램 종료\n";
    return 0;
}
#endif
