#include "ScenarioInfoPrinter.h"
#include <iostream>

namespace sm {
    void ScenarioInfoPrinter::printInfo(const ScenarioManager& manager) const {
        const auto& info = manager.getScenarioInfo();
        std::cout << u8"\n[시나리오 정보]\n";
        std::cout << u8"식별자: " << info.scenario_id << "\n";
        std::cout << u8"제목: " << info.scenario_title << "\n";
    }

    void ScenarioInfoPrinter::printBattery(const ScenarioManager& manager) const {
        const auto& battery = manager.getBatteryLocation();
        std::cout << u8"\n[포대 위치]\n";
        std::cout << u8"위도: " << battery.latitude << "\n";
        std::cout << u8"경도: " << battery.longitude << "\n";
        std::cout << u8"고도: " << battery.altitude << "\n";
    }

    void ScenarioInfoPrinter::printAircraftList(const ScenarioManager& manager) const {
        const auto& aircrafts = manager.getAircraftList();
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

    // 전체 출력 함수 (선택적 사용)
    void ScenarioInfoPrinter::printAll(const ScenarioManager& manager) const {
        printInfo(manager);
        printBattery(manager);
        printAircraftList(manager);
    }
}
