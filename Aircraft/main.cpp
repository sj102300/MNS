#include "AircraftCoordinate.h"
#include "ShootDownCheck.h"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <chrono>

struct AircraftInfo {
    std::string id;
    std::pair<double, double> startPoint;
    std::pair<double, double> finishPoint;
    std::pair<double, double> currentPoint;
    char IFF;
};

struct MissileInfo {
    std::string id;
    std::pair<double, double> MissilePoint;
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
    MissileInfo missile;
    try {
        AircraftInfo aircraft = parseArguments(argc, argv);

        AircraftCoordinate Aircraft;
        ShootDownCheck sDown;

        std::vector<double> startop = Aircraft.makeStartOpt(aircraft.startPoint, aircraft.finishPoint);
        Aircraft.initializeMultiSocket();
        sDown.initializeMultiSocket();

        while (true) { //격추 안될 시, 끝나는 로직 추가 필요
            aircraft.currentPoint = Aircraft.makePoint(startop[0], startop[1], aircraft.currentPoint);
            Aircraft.sendAircraftInfo(aircraft.currentPoint, aircraft.id, aircraft.IFF);
            
            missile.MissilePoint = sDown.MissileReciever();
            missile.id = sDown.MissileReciever(missile.id);
            if(sDown.ShootDown(aircraft.currentPoint, missile.MissilePoint)){
                sDown.sendSuccessInfo(aircraft.id, missile.id);
                cout << "격추완료 exe. 종료";
                break; //break 대시 exe파일 종료 코드 넣으면 될 듯
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cin.get();
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
