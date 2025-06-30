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

        // ���� ID �ο�
        std::string id = "MSS-" + std::to_string(100 + i);
        missile->setMissileId(id);  // �̸� ������ setter

        // Multicast, Controller ���� �� �ʱ�ȭ
        auto sender = std::make_shared<UdpMulticast>();
        sender->init("239.0.0.1", 9001);
        auto controller = std::make_shared<MissileController>();

        missile->init(sender, controller);
        missile->start(2000.0f);

        // map �� vector�� ����
        missileMap[id] = missile;
        missiles.push_back(missile);
    }

    auto receiver = std::make_shared<UdpReceiver>();
    receiver->init("239.0.0.1", 9001);  // ���� ��Ʈ �� ���ε� �ּ� ����
    receiver->setMissileMap(missileMap);
    receiver->start();  // ���� ������ ����

    std::cout << "�̻��� ���� ��� ��...\n";
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    // ���α׷� ���� ���� �����带 �����ؾ� �� ���
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

// === ���� �� ===
const std::string SUBSYSTEM_ID = "MSS";
const std::string SCN_LOCAL_IP = "http://192.168.2.22:8080";  // ���� �ּ�
const std::string SCN_SERVER_IP = "http://192.168.2.30:8080";  // SCN ���� �ּ�
const std::string UDP_IP = "239.0.0.1";        // ��Ƽĳ���� IP
const int         UDP_PORT = 9000;             // ��Ƽĳ���� ��Ʈ

// === �۷ι� ����ȭ ���� ===
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> running = false;
std::atomic<bool> isChanged = false;

//running -> �ùķ��̼� ��
//         -> quit
//running -> �ùķ��̼� ���
//          -> start

std::unordered_map<std::string, std::shared_ptr<Missile>> missileMap;
std::vector<std::shared_ptr<Missile>> missiles;
std::shared_ptr<UdpReceiver> receiver;

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

void handleQuit() {         //MFR->SM����
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���� ��ȣ ����\n";

    if (!running) {
        std::cout << u8"[" << SUBSYSTEM_ID << u8"] �ó����� ��� �� - quit ����\n";
        return;
    }

    running = false;
    isChanged = true;
    cv.notify_one();
    std::cout << u8"[" << SUBSYSTEM_ID << u8"] Ž�� ���� �Ϸ�, ���� ���� ��ȣ ��� ��...\n";
}

#include "DestroyedAircrafts.h"
// === main �Լ� ���� ===
int main() {
    ScenarioManager scenarioRunner(
        SCN_LOCAL_IP,     // ���� �ּ�
        SCN_SERVER_IP,    // SCN ���� �ּ�
        SUBSYSTEM_ID         // Ŭ���̾�Ʈ ID
    );

    scenarioRunner.setOnReadyCallback(handleStart);
    scenarioRunner.setOnQuitCallback(handleQuit);
    DestroyedAircraftsTracker* destroyedAircraftTracker = new DestroyedAircraftsTracker();

    std::thread scenarioThread([&]() {
        scenarioRunner.run();  // blocking
        });
    std::cout << u8"�ó����� ��ȣ �ޱ���..?" << "\n";
    while (true) {
        // Start Ȥ�� Quit ��ȣ ���
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return isChanged.load(); });
            isChanged = false;
        }

        if (running) {
            // ���� ��ġ ȹ��
            Coordinate battery = scenarioRunner.getBatteryLocation();
            Location loca = { battery.latitude,battery.longitude,battery.altitude };
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���� ��ġ: "
                << battery.latitude << ", " << battery.longitude << ", " << battery.altitude << "\n";

            // ���� �̻��� ����
            missiles.clear();
            missileMap.clear();

            // �̻��� ���� -- �ִ� �� ������
            for (int i = 0; i < 55; ++i) {
                auto missile = std::make_shared<Missile>();

                std::string id = "MSS-" + std::to_string(100 + i);
                missile->setMissileId(id);
                missile->setLoc(loca);

                auto sender = std::make_shared<UdpMulticast>();
                sender->init(UDP_IP, UDP_PORT);

                auto controller = std::make_shared<MissileController>(destroyedAircraftTracker);
                missile->init(sender, controller);
                missile->start(2.0); // �ʼ� 2km

                missileMap[id] = missile;
                missiles.push_back(missile);
            }

            // ���ű� ����
            receiver = std::make_shared<UdpReceiver>(destroyedAircraftTracker);
            receiver->init(UDP_IP, UDP_PORT);
            receiver->setMissileMap(missileMap);
            receiver->start();

            // MissileController���� Aircraft_map ������ ����
            for (auto& [id, missile] : missileMap) {
                auto controller = missile->getController();  // get �Լ� �ʿ�
                controller->setAircraftMap(receiver->getAircraftMapPtr());  // get �Լ� �ʿ�
            }
        }
        else {
            std::cout << u8"[" << SUBSYSTEM_ID << u8"] �ùķ��̼� ���� ó�� ��...\n";

            // ���ű� ����
            if (receiver) receiver->close();

            // �̻��� ����
            for (auto& m : missiles) {
                m->stop();
            }

			destroyedAircraftTracker->clearDestroyedAircrafts();  // ���ߵ� �װ��� ��� �ʱ�ȭ
        }
    }

    if (scenarioThread.joinable()) scenarioThread.join();

    std::cout << u8"[" << SUBSYSTEM_ID << u8"] ���α׷� ����\n";
    return 0;
}
#endif