// ScenarioTypes.h
#pragma once

#include <string>
#include <vector>

class ScenarioInfo {
public:
    std::string scenario_id;  // �ó����� �ĺ���
    std::string scenario_title;  // �ó����� ����

    ScenarioInfo() = default;
    ScenarioInfo(const std::string& id, const std::string& title)
        : scenario_id(id), scenario_title(title) {
    }
};

class Coordinate {
public:
    double latitude = 0.0;  // ����
    double longitude = 0.0;  // �浵
    double altitude = 0.0;  // ��

    Coordinate() = default;
    Coordinate(double lat, double lon, double alt)
        : latitude(lat), longitude(lon), altitude(alt) {
    }
};

class AircraftInfo {
public:
    std::string aircraft_id;  // �װ��� �ĺ���
    std::string friend_or_foe;  // �Ǿ�����
    Coordinate start_point;  // ���� - ��ǥ
    Coordinate end_point;  // ���� - ��ǥ

    AircraftInfo() = default;
    AircraftInfo(const std::string& id, const std::string& foe_type,
        const Coordinate& start, const Coordinate& end)
        : aircraft_id(id), friend_or_foe(foe_type),
        start_point(start), end_point(end) {
    }
};
