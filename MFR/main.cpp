#include "ScenarioInit.h"
#include "ScenarioManager.h"

int main() {
    ScenarioInit scenarioRunner(
        "http://127.0.0.1:9015",  // 내 시나리오 수신 주소
        "http://127.0.0.1:8000",  // 요청할 SCN 서버 주소
        "MFR"                     // 내 클라이언트 ID
    );
    scenarioRunner.run();


    std::cout << "\n" << u8"===============================================" << "\n";

    //ScenarioInfo info = scenarioRunner.getScenarioInfo();
    //std::cout << u8"시나리오 ID는: " << info.scenario_id << std::endl;

    //Coordinate battery = scenarioRunner.getBatteryLocation();
    //std::cout << u8"[포대 위치] 위도: " << battery.latitude
    //    << u8", 경도: " << battery.longitude
    //    << u8", 고도: " << battery.altitude << std::endl;

    //std::vector<AircraftInfo> aircrafts = scenarioRunner.getAircraftList();
    //for (const auto& ac : aircrafts) {
    //    std::cout << u8"[항공기] ID: " << ac.aircraft_id << u8", 피아: " << ac.friend_or_foe << std::endl;
    //}

    std::cin.get(); // 프로그램 종료 임시 방지
    return 0;
}
