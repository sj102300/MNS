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

struct AircraftInfo {
    std::string id;
    std::pair<double, double> startPoint;
    std::pair<double, double> finishPoint;
    std::pair<double, double> currentPoint;
    char IFF;
};

struct MissileInfo {
    std::string id;
    std::pair<double, double> currentPoint;
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

        // 서버없이 테스트 하기 위한 더미데이터
        /*
        AircraftInfo aircraft;
        aircraft.id = "ATS-0001";
        aircraft.startPoint = { 2,2 };
        aircraft.finishPoint = { 10,10 };
        aircraft.currentPoint = aircraft.startPoint;
        aircraft.IFF = 'E';
        */

        AircraftCoordinate Aircraft;

        std::vector<double> startop = Aircraft.makeStartOpt(aircraft.startPoint, aircraft.finishPoint);
        Aircraft.initializeMultiSocket();

        UdpMulticastReceiver receiver;
        if (!receiver.init("239.0.0.1", 9000)) {
            return -1;
        }

        while (true) { //격추 안될 시, 끝나는 로직 추가 필요 => endPoint를 무조건 지나가게 됨 => 이걸 이용하자 => 해결 완 ~
            aircraft.currentPoint = Aircraft.makePoint(startop[0], startop[1], aircraft.currentPoint);
            Aircraft.sendAircraftInfo(aircraft.currentPoint, aircraft.id, aircraft.IFF);
            if (Aircraft.SurviveStatus(aircraft.currentPoint, aircraft.finishPoint)) exit(0);
            
            //미사일 격추 필요
            //미사일 id, point에 각각의 값 넣기 -> 항공기의 currentPoint와 비교 -> 격추판정 (격추되면 성공정보 보내고 프로세스 종료)
            ParsedMissileData missile = receiver.receiveMissile();
            pair<double, double> MissilePoints = {missile.latitude, missile.longitude};
            //미사일 통신 테스트용
            if (missile.missileId.empty()) {
                std::cerr << "미사일 데이터 없음 (missileId 비어 있음)\n";
            }
            else {
                cout << "missile : " << MissilePoints.first << ", " << MissilePoints.second << endl;
            }
            if (ShootDowns(aircraft.currentPoint, MissilePoints)) {
                // 격추성공 통신코드 넣어야 함 + 프로세스 종료해야 함
                initializeMultiSenderSocket();
                sendSuccessInfo(aircraft.id, missile.missileId);
                while (true) {
                    // 격추판단 테스트용
                    sendSuccessInfo(aircraft.id, missile.missileId);
                    cout << "격추완료" << endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    //
                }
                //exit(0);
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
