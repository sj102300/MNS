#if 0
#pragma once
#include "UdpReceiver.h"
#include "UdpMuticast.h"
#include "MissileController.h"

#include <thread>
#include <vector>

int main() {

    std::unordered_map<std::string, std::shared_ptr<Missile>> missileMap;
    std::vector<std::shared_ptr<Missile>> missiles;

    for (int i = 0; i < 6; ++i) {
        auto missile = std::make_shared<Missile>();

        // 고유 ID 부여
        std::string id = "MSS-" + std::to_string(100 + i);
        missile->setMissileId(id);  // 미리 정의한 setter

        // Multicast, Controller 생성 및 초기화
        auto sender = std::make_shared<UdpMulticast>();
        sender->init("239.0.0.1", 9001);
        auto controller = std::make_shared<MissileController>();

        missile->init(sender, controller);
        missile->start(2000.0f);

        // map 및 vector에 저장
        missileMap[id] = missile;
        missiles.push_back(missile);
    }

    auto receiver = std::make_shared<UdpReceiver>();
    receiver->init("239.0.0.1", 9001);  // 수신 포트 및 바인드 주소 설정
    receiver->setMissileMap(missileMap);
    receiver->start();  // 수신 쓰레드 시작

    std::cout << "미사일 수신 대기 중...\n";
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    // 프로그램 종료 전에 쓰레드를 종료해야 할 경우
    std::cout << "Receiver and Sender threads completed.\n";

    return 0;
}
#endif
#if 1
#pragma once
#include "UdpReceiver.h"
#include "UdpMuticast.h"
#include "MissileController.h"
#include "ScenarioManager.h"

#include <vector>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#pragma comment(lib, "ScenarioManager.lib")


using namespace sm;

// === 설정 값 ===
const std::string SUBSYSTEM_ID = "MSS";
const std::string SCN_LOCAL_IP = "http://192.168.2.22:8080";  // 수신 주소
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

std::unordered_map<std::string, std::shared_ptr<Missile>> missileMap;
std::vector<std::shared_ptr<Missile>> missiles;
std::shared_ptr<UdpReceiver> receiver;

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

#include "DestroyedAircrafts.h"
// === main 함수 정의 ===
int main() {
    ScenarioManager scenarioRunner(
        SCN_LOCAL_IP,     // 수신 주소
        SCN_SERVER_IP,    // SCN 서버 주소
        SUBSYSTEM_ID         // 클라이언트 ID
    );

    scenarioRunner.setOnReadyCallback(handleStart);
    scenarioRunner.setOnQuitCallback(handleQuit);
    DestroyedAircraftsTracker* destroyedAircraftTracker = new DestroyedAircraftsTracker();

    std::thread scenarioThread([&]() {
        scenarioRunner.run();  // blocking
        });
    std::cout << u8"시나리오 신호 받기전..?" << "\n";
    while (true) {
        // Start 혹은 Quit 신호 대기
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }

        if (running) {
            // 포대 위치 획득
            Coordinate battery = scenarioRunner.getBatteryLocation();
            Location loca = { battery.latitude,battery.longitude,battery.altitude };
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] 포대 위치: "
                << battery.latitude << ", " << battery.longitude << ", " << battery.altitude << "\n";

            // 기존 미사일 정리
            missiles.clear();
            missileMap.clear();

            // 미사일 생성 -- 최대 몇 발인지
            for (int i = 0; i < 55; ++i) {
                auto missile = std::make_shared<Missile>();

                std::string id = "MSS-" + std::to_string(100 + i);
                missile->setMissileId(id);
                missile->setLoc(loca);

                auto sender = std::make_shared<UdpMulticast>();
                sender->init(UDP_IP, UDP_PORT);

                auto controller = std::make_shared<MissileController>(destroyedAircraftTracker);
                missile->init(sender, controller);
                missile->start(2.0); // 초속 2km

                missileMap[id] = missile;
                missiles.push_back(missile);
            }

            // 수신기 시작
            receiver = std::make_shared<UdpReceiver>(destroyedAircraftTracker);
            receiver->init(UDP_IP, UDP_PORT);
            receiver->setMissileMap(missileMap);
            receiver->start();

            // MissileController에도 Aircraft_map 포인터 설정
            for (auto& [id, missile] : missileMap) {
                auto controller = missile->getController();  // get 함수 필요
                controller->setAircraftMap(receiver->getAircraftMapPtr());  // get 함수 필요
            }
        }
        else {
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] 시뮬레이션 종료 처리 중...\n";

            // 수신기 종료
            if (receiver) receiver->close();

            // 미사일 종료
            for (auto& m : missiles) {
                m->stop();
            }

			destroyedAircraftTracker->clearDestroyedAircrafts();  // 격추된 항공기 목록 초기화
        }
    }

    if (scenarioThread.joinable()) scenarioThread.join();

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 프로그램 종료\n";
    return 0;
}
#endif