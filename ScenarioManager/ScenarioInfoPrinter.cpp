#include "ScenarioInfoPrinter.h"
#include <iostream>

namespace sm {
    void ScenarioInfoPrinter::printInfo(const HttpClient& client) const {
        const auto& info = client.getScenarioInfo();
        std::cout << u8"\n[�ó����� ����]\n";
        std::cout << u8"�ĺ���: " << info.scenario_id << "\n";
        std::cout << u8"����: " << info.scenario_title << "\n";
    }

    void ScenarioInfoPrinter::printBattery(const HttpClient& client) const {
        const auto& battery = client.getBatteryLocation();
        std::cout << u8"\n[���� ��ġ]\n";
        std::cout << u8"����: " << battery.latitude << "\n";
        std::cout << u8"�浵: " << battery.longitude << "\n";
        std::cout << u8"��: " << battery.altitude << "\n";
    }

    void ScenarioInfoPrinter::printAircraftList(const HttpClient& client) const {
        const auto& aircrafts = client.getAircraftList();
        std::cout << u8"\n[�װ��� ���]\n";
        for (size_t i = 0; i < aircrafts.size(); ++i) {
            const auto& ac = aircrafts[i];
            std::cout << u8"\n�װ��� " << i + 1 << "\n";
            std::cout << u8"�ĺ���: " << ac.aircraft_id << "\n";
            std::cout << u8"�Ǿ�����: " << ac.friend_or_foe << "\n";
            std::cout << u8"[����] ����: " << ac.start_point.latitude
                << u8" �浵: " << ac.start_point.longitude
                << u8" ��: " << ac.start_point.altitude << "\n";
            std::cout << u8"[����] ����: " << ac.end_point.latitude
                << u8" �浵: " << ac.end_point.longitude
                << u8" ��: " << ac.end_point.altitude << "\n";
        }
    }

    void ScenarioInfoPrinter::printAll(const HttpClient& client) const {
        printInfo(client);
        printBattery(client);
        printAircraftList(client);
    }
}