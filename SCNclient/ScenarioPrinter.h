#pragma once

#include "JsonParser.h"
#include <iostream>
#include <vector>

inline void print_scenario_info(const ScenarioInfo& info) {
	std::cout << u8"\n[�ó����� ����]\n";
	std::cout << u8"�ĺ���: " << info.scenario_id << "\n";
	std::cout << u8"����: " << info.scenario_title << "\n";
}

inline void print_battery_location(const Coordinate& battery) {
	std::cout << u8"\n[���� ��ġ]\n";
	std::cout << u8"����: " << battery.latitude << "\n";
	std::cout << u8"�浵: " << battery.longitude << "\n";
	std::cout << u8"��: " << battery.altitude << "\n";
}

inline void print_aircraft_list(const std::vector<AircraftInfo>& aircraft_list) {
	std::cout << u8"\n[�װ��� ���]\n";
	for (size_t i = 0; i < aircraft_list.size(); ++i) {
		const auto& ac = aircraft_list[i];
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
