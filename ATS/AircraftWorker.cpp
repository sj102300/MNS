#include "Missile.h"               // �ݵ�� �ֻ�ܿ�
#include "AircraftWorker.h"
#include "UdpMulticastReceiver.h"
#include <chrono>
#include <iostream>
using namespace ats;

AircraftWorker::AircraftWorker(AircraftInfo info)
    : info_(info), running_(true) {
}

void AircraftWorker::stop() {
    running_ = false;
}

void AircraftWorker::operator()() {
    AircraftCoordinate coord;
    auto direction = coord.makeStartOpt(info_.startPoint, info_.finishPoint);
    coord.initializeMultiSocket();

    UdpMulticastReceiver receiver;
    if (!receiver.init("239.0.0.1", 9000)) {
        std::cerr << "[ERROR] Receiver init failed\n";
        return;
    }

    while (running_) {
        info_.currentPoint = coord.makePoint(direction[0], direction[1], info_.currentPoint);
        coord.sendAircraftInfo(info_.currentPoint, info_.id, info_.IFF);

        if (coord.SurviveStatus(info_.currentPoint, info_.finishPoint)) {
            //std::cout << "[���� ����] " << info_.id << std::endl;
            break;
        }

        auto missiles = receiver.receiveAllMissiles();
        for (const auto& [mid, missile] : missiles) {
            std::pair<double, double> mpos = { missile.latitude, missile.longitude };

            if (ShootDowns(info_.currentPoint, mpos)) {
                sendSuccessInfo(info_.id, missile.missileId);
                //std::cout << "[���� ����] " << info_.id << std::endl;
                return;
            }

        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    //std::cout << "[������ ����] " << info_.id << std::endl;
}
