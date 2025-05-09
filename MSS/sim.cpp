#include "ScenarioParser.hpp"
#include <iostream>

int main() {
    Scenario scenario = parseScenarioFromFile("example.json");

    cout << "�ó����� ID: " << scenario.scenario_id <<"\n\n";
    for (const auto& ac : scenario.aircraft_list) {
        cout << "  �װ���: " << ac.id << " (" << ac.friend_or_foe << ")\n";
        cout << "  �װ��� ������\n";
        cout << "  ��: " << ac.start_point.altitude << " ����: " << ac.start_point.latitude << " �浵: " << ac.start_point.longitude << endl;
        cout << "  �װ��� ����\n";
        cout << "  ��: " << ac.end_point.altitude << " ����: " << ac.end_point.latitude << " �浵: " << ac.end_point.longitude << "\n\n";
    }

    return 0;
}