#include "MissileReceiverThread.h"
#include "UdpMulticastReceiver.h"
#include "GlobalMissileStore.h"  // 전역 미사일 저장소 (mutex 포함)

#include <thread>
#include <chrono>
#include <iostream>

void startMissileReceiverThread() {
    std::thread([]() {
        UdpMulticastReceiver receiver;
        if (!receiver.init("239.0.0.1", 9000)) {
            std::cerr << "[ERROR] Missile receiver init failed\n";
            return;
        }

        while (true) {
            std::unordered_map<std::string, ParsedMissileData> latest;
            receiver.receiveAllMissiles(latest);

            {
                globalMissiles = std::move(latest);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        }).detach();
}
