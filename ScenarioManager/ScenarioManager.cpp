#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioManager.h"

#include <cpprest/http_client.h>
#include <windows.h>
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::client;

ScenarioManager::ScenarioManager(const std::string& url)
    : server_url_(url) {
}

bool ScenarioManager::requestScenario(const std::string& scenario_id) {
    http_client client(utility::conversions::to_string_t(server_url_));

    try {
        http_request req(methods::POST);
        req.set_request_uri(U("/scenario/info"));
        req.headers().set_content_type(U("application/json"));

        json::value postData;
        postData[U("scenario_id")] = json::value::string(utility::conversions::to_string_t(scenario_id));
        req.set_body(postData);

        http_response response = client.request(req).get();
        if (response.status_code() != status_codes::OK) {
            std::cerr << u8"[ScenarioManager] 요청 실패 - HTTP " << response.status_code() << std::endl;
            return false;
        }

        auto root = response.extract_json().get();
        if (!root.has_field(U("scenario_id")) || !root.has_field(U("battery_location")) || !root.has_field(U("aircraft_list"))) {
            std::cerr << u8"[ScenarioManager] 응답 JSON 구조 오류\n";
            return false;
        }

        // 필요한 부분만 on/off
        scenario_info_ = parseScenarioInfo(root);
        battery_location_ = parseBatteryLocation(root);
        aircraft_list_ = parseAircraftList(root);

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << u8"[ScenarioManager] 예외 발생: " << e.what() << std::endl;
        return false;
    }
}

ScenarioInfo ScenarioManager::getScenarioInfo() const { return scenario_info_; }
Coordinate ScenarioManager::getBatteryLocation() const { return battery_location_; }
std::vector<AircraftInfo> ScenarioManager::getAircraftList() const { return aircraft_list_; }

std::string ScenarioManager::to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
    return result;
}

ScenarioInfo ScenarioManager::parseScenarioInfo(const json::value& root) {
    return ScenarioInfo(
        to_utf8(root.at(U("scenario_id")).as_string()),
        to_utf8(root.at(U("scenario_title")).as_string())
    );
}

Coordinate ScenarioManager::parseBatteryLocation(const json::value& root) {
    auto obj = root.at(U("battery_location")).as_object();
    return Coordinate(
        obj.at(U("latitude")).as_double(),
        obj.at(U("longitude")).as_double(),
        obj.at(U("altitude")).as_double()
    );
}

std::vector<AircraftInfo> ScenarioManager::parseAircraftList(const json::value& root) {
    std::vector<AircraftInfo> result;
    for (const auto& item : root.at(U("aircraft_list")).as_array()) {
        auto a = item.as_object();
        auto sp = a.at(U("start_point")).as_object();
        auto ep = a.at(U("end_point")).as_object();
        result.emplace_back(
            to_utf8(a.at(U("aircraft_id")).as_string()),
            to_utf8(a.at(U("friend_or_foe")).as_string()),
            Coordinate(sp.at(U("latitude")).as_double(), sp.at(U("longitude")).as_double(), sp.at(U("altitude")).as_double()),
            Coordinate(ep.at(U("latitude")).as_double(), ep.at(U("longitude")).as_double(), ep.at(U("altitude")).as_double())
        );
    }
    return result;
}
