#include "AircraftCoordinate.h"
#include "UdpMulticastReceiver.h"
#include "Missile.h"
#include "ShootDown.h"
#include "sender.h"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <chrono>
#include <unordered_map>

struct AircraftInfo {
    std::string id;
    std::pair<double, double> startPoint;
    std::pair<double, double> finishPoint;
    std::pair<double, double> currentPoint;
    char IFF;
};

AircraftInfo parseArguments(int argc, char* argv[]) {
    if (argc < 8) {
        throw std::invalid_argument("올바른 시나리오 형식을 수신받지 못 했습니다.");
    }

    AircraftInfo info;
    info.id = argv[1];
    info.startPoint = { std::stod(argv[2]), std::stod(argv[3]) };
    info.finishPoint = { std::stod(argv[5]), std::stod(argv[6]) };
    info.currentPoint = info.startPoint;
    info.IFF = argv[7][0];

    return info;
}

int main(int argc, char* argv[]) {
    try {
        AircraftInfo aircraft = parseArguments(argc, argv);

        AircraftCoordinate Aircraft;
        std::vector<double> startop = Aircraft.makeStartOpt(aircraft.startPoint, aircraft.finishPoint);
        Aircraft.initializeMultiSocket();

        UdpMulticastReceiver receiver;
        if (!receiver.init("239.0.0.1", 9000)) {
            return -1;
        }

        while (true) {
            // 항공기 위치 이동 및 전송
            aircraft.currentPoint = Aircraft.makePoint(startop[0], startop[1], aircraft.currentPoint);
            Aircraft.sendAircraftInfo(aircraft.currentPoint, aircraft.id, aircraft.IFF);

            // 도착 지점 도달 시 종료
            if (Aircraft.SurviveStatus(aircraft.currentPoint, aircraft.finishPoint)) exit(0);

            // 미사일 수신 및 격추 판정
            auto missileMap = receiver.receiveAllMissiles();

            for (const auto& [id, missile] : missileMap) {
                std::pair<double, double> MissilePoints = { missile.latitude, missile.longitude };

                std::cout << "[미사일 수신] ID: " << id
                    << " 위치: (" << MissilePoints.first << ", " << MissilePoints.second << ")\n";

                if (ShootDowns(aircraft.currentPoint, MissilePoints)) {
                    initializeMultiSenderSocket();
                    sendSuccessInfo(aircraft.id, missile.missileId);
                    std::exit(0);

                    /*while (true) {
                        sendSuccessInfo(aircraft.id, missile.missileId);
                        std::cout << "격추완료\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    }*/
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
