#include "ScenarioInfoPrinter.h"
#include <iostream>

namespace sm {
    void ScenarioInfoPrinter::printInfo(const HttpClient& client) const {
        const auto& info = client.getScenarioInfo();
        std::cout << u8"\n[시나리오 정보]\n";
        std::cout << u8"식별자: " << info.scenario_id << "\n";
        std::cout << u8"제목: " << info.scenario_title << "\n";
    }

    void ScenarioInfoPrinter::printBattery(const HttpClient& client) const {
        const auto& battery = client.getBatteryLocation();
        std::cout << u8"\n[포대 위치]\n";
        std::cout << u8"위도: " << battery.latitude << "\n";
        std::cout << u8"경도: " << battery.longitude << "\n";
        std::cout << u8"고도: " << battery.altitude << "\n";
    }

    void ScenarioInfoPrinter::printAircraftList(const HttpClient& client) const {
        const auto& aircrafts = client.getAircraftList();
        std::cout << u8"\n[항공기 목록]\n";
        for (size_t i = 0; i < aircrafts.size(); ++i) {
            const auto& ac = aircrafts[i];
            std::cout << u8"\n항공기 " << i + 1 << "\n";
            std::cout << u8"식별자: " << ac.aircraft_id << "\n";
            std::cout << u8"피아정보: " << ac.friend_or_foe << "\n";
            std::cout << u8"[시점] 위도: " << ac.start_point.latitude
                << u8" 경도: " << ac.start_point.longitude
                << u8" 고도: " << ac.start_point.altitude << "\n";
            std::cout << u8"[종점] 위도: " << ac.end_point.latitude
                << u8" 경도: " << ac.end_point.longitude
                << u8" 고도: " << ac.end_point.altitude << "\n";
        }
    }

    void ScenarioInfoPrinter::printAll(const HttpClient& client) const {
        printInfo(client);
        printBattery(client);
        printAircraftList(client);
    }
}