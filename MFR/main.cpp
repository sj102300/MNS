#include "ScenarioInit.h"
#include "ScenarioManager.h"

int main() {
    ScenarioInit scenarioRunner(
        "http://127.0.0.1:9015",  // �� �ó����� ���� �ּ�
        "http://127.0.0.1:8000",  // ��û�� SCN ���� �ּ�
        "MFR"                     // �� Ŭ���̾�Ʈ ID
    );
    scenarioRunner.run();


    std::cout << "\n" << u8"===============================================" << "\n";

    //ScenarioInfo info = scenarioRunner.getScenarioInfo();
    //std::cout << u8"�ó����� ID��: " << info.scenario_id << std::endl;

    //Coordinate battery = scenarioRunner.getBatteryLocation();
    //std::cout << u8"[���� ��ġ] ����: " << battery.latitude
    //    << u8", �浵: " << battery.longitude
    //    << u8", ��: " << battery.altitude << std::endl;

    //std::vector<AircraftInfo> aircrafts = scenarioRunner.getAircraftList();
    //for (const auto& ac : aircrafts) {
    //    std::cout << u8"[�װ���] ID: " << ac.aircraft_id << u8", �Ǿ�: " << ac.friend_or_foe << std::endl;
    //}

    std::cin.get(); // ���α׷� ���� �ӽ� ����
    return 0;
}
