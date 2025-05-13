#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "MFR.h"
#include "GlobalState.h"
#include "JsonParser.h"
#include "ScenarioInfoPrinter.h"  //  출력 확인 안할거면 삭제 가능

#include <windows.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include <locale>

using namespace web;
using namespace web::http;
using namespace web::http::client;

// === 파서 사용 여부 설정 ===
// 필요한 것만 1로 설정하고, 사용하지 않을 항목은 0으로 비활성화

#define USE_SCENARIO_INFO    1   // 시나리오 개요 (식별자, 제목)
#define USE_BATTERY_LOCATION 1   // 포대 위치 (위경고도)
#define USE_AIRCRAFT_LIST    1   // 항공기 목록 (식별자, 피아구분, 시점, 종점)
constexpr char ADDRESS[] = "http://127.0.0.1:8000";  // client + port 설정
const utility::string_t SCENARIO_SERVER_URL = utility::conversions::to_string_t(ADDRESS);

ScenarioInfo scenario_info;
Coordinate battery_location;
std::vector<AircraftInfo> aircraft_list;

ScenarioInfo parse_scenario_info(const json::value& root);
Coordinate parse_battery_location(const json::value& root);
std::vector<AircraftInfo> parse_aircraft_list(const json::value& root);

// 출력용
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

		// scenario_id를 JSON으로 설정
		json::value postData;
		postData[U("scenario_id")] = json::value::string(utility::conversions::to_string_t(g_scenario_id));
		req.set_body(postData);

		http_response response = client.request(req).get();

		if (response.status_code() == status_codes::OK) {
			json::value scenario_json = response.extract_json().get();

			// 전체 출력 - 제대로 들어왔는지 확인
			//std::string json_utf8 = to_utf8(scenario_json.serialize());
			//std::cout << u8"[서버가 응답한 JSON 전체]\n" << json_utf8 << "\n";

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
			std::cerr << u8"시나리오 요청 실패: HTTP " << response.status_code() << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << u8"예외 발생: " << e.what() << std::endl;
	}
}
