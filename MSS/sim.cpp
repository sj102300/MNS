#include "ScenarioParser.hpp"
#include <iostream>

int main() {
    Scenario scenario = parseScenarioFromFile("example.json");

    cout << "시나리오 ID: " << scenario.scenario_id <<"\n\n";
    for (const auto& ac : scenario.aircraft_list) {
        cout << "  항공기: " << ac.id << " (" << ac.friend_or_foe << ")\n";
        cout << "  항공기 시작점\n";
        cout << "  고도: " << ac.start_point.altitude << " 위도: " << ac.start_point.latitude << " 경도: " << ac.start_point.longitude << endl;
        cout << "  항공기 종점\n";
        cout << "  고도: " << ac.end_point.altitude << " 위도: " << ac.end_point.latitude << " 경도: " << ac.end_point.longitude << "\n\n";
    }

    return 0;
}