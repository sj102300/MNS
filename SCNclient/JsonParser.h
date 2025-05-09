// ScenarioTypes.h
#pragma once

#include <string>
#include <vector>

class ScenarioInfo {
public:
    std::string scenario_id;  // 시나리오 식별자
    std::string scenario_title;  // 시나리오 제목

    ScenarioInfo() = default;
    ScenarioInfo(const std::string& id, const std::string& title)
        : scenario_id(id), scenario_title(title) {
    }
};

class Coordinate {
public:
    double latitude = 0.0;  // 위도
    double longitude = 0.0;  // 경도
    double altitude = 0.0;  // 고도

    Coordinate() = default;
    Coordinate(double lat, double lon, double alt)
        : latitude(lat), longitude(lon), altitude(alt) {
    }
};

class AircraftInfo {
public:
    std::string aircraft_id;  // 항공기 식별자
    std::string friend_or_foe;  // 피아정보
    Coordinate start_point;  // 시점 - 좌표
    Coordinate end_point;  // 종점 - 좌표

    AircraftInfo() = default;
    AircraftInfo(const std::string& id, const std::string& foe_type,
        const Coordinate& start, const Coordinate& end)
        : aircraft_id(id), friend_or_foe(foe_type),
        start_point(start), end_point(end) {
    }
};
