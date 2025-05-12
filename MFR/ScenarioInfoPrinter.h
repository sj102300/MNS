#pragma once

#include "JsonParser.h"
#include <iostream>
#include <vector>

inline void print_scenario_info(const ScenarioInfo& info) {
	std::cout << u8"\n[시나리오 정보]\n";
	std::cout << u8"식별자: " << info.scenario_id << "\n";
	std::cout << u8"제목: " << info.scenario_title << "\n";
}

inline void print_battery_location(const Coordinate& battery) {
	std::cout << u8"\n[포대 위치]\n";
	std::cout << u8"위도: " << battery.latitude << "\n";
	std::cout << u8"경도: " << battery.longitude << "\n";
	std::cout << u8"고도: " << battery.altitude << "\n";
}

inline void print_aircraft_list(const std::vector<AircraftInfo>& aircraft_list) {
	std::cout << u8"\n[항공기 목록]\n";
	for (size_t i = 0; i < aircraft_list.size(); ++i) {
		const auto& ac = aircraft_list[i];
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
