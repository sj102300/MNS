#pragma once

#include "JsonParser.h"
#include <cpprest/json.h>
#include <string>
#include <vector>

class ScenarioManager {
public:
    explicit ScenarioManager(const std::string& server_url);

    bool requestScenario(const std::string& scenario_id);

    ScenarioInfo getScenarioInfo() const;
    Coordinate getBatteryLocation() const;
    std::vector<AircraftInfo> getAircraftList() const;

private:
    std::string server_url_;
    ScenarioInfo scenario_info_;
    Coordinate battery_location_;
    std::vector<AircraftInfo> aircraft_list_;

    // 내부 파싱 함수들
    std::string to_utf8(const std::wstring& wstr);
    ScenarioInfo parseScenarioInfo(const web::json::value& root);
    Coordinate parseBatteryLocation(const web::json::value& root);
    std::vector<AircraftInfo> parseAircraftList(const web::json::value& root);
};
