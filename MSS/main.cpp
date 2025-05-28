#if 1
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
