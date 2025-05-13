#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "MFR.h"
#include "GlobalState.h"
#include "JsonParser.h"
#include "ScenarioInfoPrinter.h"  //  ��� Ȯ�� ���ҰŸ� ���� ����

#include <windows.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include <locale>

using namespace web;
using namespace web::http;
using namespace web::http::client;

// === �ļ� ��� ���� ���� ===
// �ʿ��� �͸� 1�� �����ϰ�, ������� ���� �׸��� 0���� ��Ȱ��ȭ

#define USE_SCENARIO_INFO    1   // �ó����� ���� (�ĺ���, ����)
#define USE_BATTERY_LOCATION 1   // ���� ��ġ (�����)
#define USE_AIRCRAFT_LIST    1   // �װ��� ��� (�ĺ���, �ǾƱ���, ����, ����)
constexpr char ADDRESS[] = "http://127.0.0.1:8000";  // client + port ����
const utility::string_t SCENARIO_SERVER_URL = utility::conversions::to_string_t(ADDRESS);

ScenarioInfo scenario_info;
Coordinate battery_location;
std::vector<AircraftInfo> aircraft_list;

ScenarioInfo parse_scenario_info(const json::value& root);
Coordinate parse_battery_location(const json::value& root);
std::vector<AircraftInfo> parse_aircraft_list(const json::value& root);

// ��¿�
std::string to_utf8(const std::wstring& wstr) {
	if (wstr.empty()) return {};
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
	return result;
}

ScenarioInfo parse_scenario_info(const json::value& root) {
	return ScenarioInfo(
		to_utf8(root.at(U("scenario_id")).as_string()),
		to_utf8(root.at(U("scenario_title")).as_string())
	);
}

Coordinate parse_battery_location(const json::value& root) {
	auto battery = root.at(U("battery_location")).as_object();
	return Coordinate(
		battery.at(U("latitude")).as_double(),
		battery.at(U("longitude")).as_double(),
		battery.at(U("altitude")).as_double()
	);
}

std::vector<AircraftInfo> parse_aircraft_list(const json::value& root) {
	std::vector<AircraftInfo> result;
	auto aircraft_array = root.at(U("aircraft_list")).as_array();

	for (const auto& item : aircraft_array) {
		auto a = item.as_object();
		auto sp = a.at(U("start_point")).as_object();
		auto ep = a.at(U("end_point")).as_object();

		result.emplace_back(
			to_utf8(a.at(U("aircraft_id")).as_string()),
			to_utf8(a.at(U("friend_or_foe")).as_string()),
			Coordinate(
				sp.at(U("latitude")).as_double(),
				sp.at(U("longitude")).as_double(),
				sp.at(U("altitude")).as_double()
			),
			Coordinate(
				ep.at(U("latitude")).as_double(),
				ep.at(U("longitude")).as_double(),
				ep.at(U("altitude")).as_double()
			)
		);
	}
	return result;
}

void request_scenario() {
	http_client client(SCENARIO_SERVER_URL);

	try {
		http_request req(methods::POST);
		req.set_request_uri(U("/scenario/detail"));
		req.headers().set_content_type(U("application/json"));

		// scenario_id�� JSON���� ����
		json::value postData;
		postData[U("scenario_id")] = json::value::string(utility::conversions::to_string_t(g_scenario_id));
		req.set_body(postData);

		http_response response = client.request(req).get();

		if (response.status_code() == status_codes::OK) {
			json::value scenario_json = response.extract_json().get();

			// ��ü ��� - ����� ���Դ��� Ȯ��
			//std::string json_utf8 = to_utf8(scenario_json.serialize());
			//std::cout << u8"[������ ������ JSON ��ü]\n" << json_utf8 << "\n";

#if USE_SCENARIO_INFO
			ScenarioInfo scenario_info = parse_scenario_info(scenario_json);
			print_scenario_info(scenario_info);
#endif

#if USE_BATTERY_LOCATION
			Coordinate battery_location = parse_battery_location(scenario_json);
			print_battery_location(battery_location);
#endif

#if USE_AIRCRAFT_LIST
			std::vector<AircraftInfo> aircraft_list = parse_aircraft_list(scenario_json);
			print_aircraft_list(aircraft_list);
#endif
		}
		else {
			std::cerr << u8"�ó����� ��û ����: HTTP " << response.status_code() << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << u8"���� �߻�: " << e.what() << std::endl;
	}
}
