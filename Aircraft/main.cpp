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
        throw std::invalid_argument("�ùٸ� �ó����� ������ ���Ź��� �� �߽��ϴ�.");
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

        while (true) { //���� �ȵ� ��, ������ ���� �߰� �ʿ�
            aircraft.currentPoint = Aircraft.makePoint(startop[0], startop[1], aircraft.currentPoint);
            Aircraft.sendAircraftInfo(aircraft.currentPoint, aircraft.id, aircraft.IFF);
            
            missile.MissilePoint = sDown.MissileReciever();
            missile.id = sDown.MissileReciever(missile.id);
            if(sDown.ShootDown(aircraft.currentPoint, missile.MissilePoint)){
                sDown.sendSuccessInfo(aircraft.id, missile.id);
                cout << "���߿Ϸ� exe. ����";
                break; //break ��� exe���� ���� �ڵ� ������ �� ��
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
