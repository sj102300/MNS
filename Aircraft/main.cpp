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
    try {
        AircraftInfo aircraft = parseArguments(argc, argv);

        // �������� �׽�Ʈ �ϱ� ���� ���̵�����
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

        while (true) { //���� �ȵ� ��, ������ ���� �߰� �ʿ� => endPoint�� ������ �������� �� => �̰� �̿����� => �ذ� �� ~
            aircraft.currentPoint = Aircraft.makePoint(startop[0], startop[1], aircraft.currentPoint);
            Aircraft.sendAircraftInfo(aircraft.currentPoint, aircraft.id, aircraft.IFF);
            if (Aircraft.SurviveStatus(aircraft.currentPoint, aircraft.finishPoint)) exit(0);
            
            //�̻��� ���� �ʿ�
            //�̻��� id, point�� ������ �� �ֱ� -> �װ����� currentPoint�� �� -> �������� (���ߵǸ� �������� ������ ���μ��� ����)
            ParsedMissileData missile = receiver.receiveMissile();
            pair<double, double> MissilePoints = {missile.latitude, missile.longitude};
            //�̻��� ��� �׽�Ʈ��
            if (missile.missileId.empty()) {
                std::cerr << "�̻��� ������ ���� (missileId ��� ����)\n";
            }
            else {
                cout << "missile : " << MissilePoints.first << ", " << MissilePoints.second << endl;
            }
            if (ShootDowns(aircraft.currentPoint, MissilePoints)) {
                // ���߼��� ����ڵ� �־�� �� + ���μ��� �����ؾ� ��
                initializeMultiSenderSocket();
                sendSuccessInfo(aircraft.id, missile.missileId);
                while (true) {
                    // �����Ǵ� �׽�Ʈ��
                    sendSuccessInfo(aircraft.id, missile.missileId);
                    cout << "���߿Ϸ�" << endl;
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
