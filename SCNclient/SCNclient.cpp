#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCNclient.h"

#include <windows.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include <locale>

using namespace web;
using namespace web::http;
using namespace web::http::client;

std::string to_utf8(const std::wstring& wstr) {
	if (wstr.empty()) return {};
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
	return result;
}

void request_scenario() {
	http_client client(U("http://127.0.0.1:8000"));

	try {
		http_request req(methods::POST);
		req.set_request_uri(U("/scenario/detail"));
		req.headers().set_content_type(U("application/json"));
		req.set_body(U("{}"));

		http_response response = client.request(req).get();

		if (response.status_code() == status_codes::OK) {
			json::value scenario_json = response.extract_json().get();

			std::string json_utf8 = to_utf8(scenario_json.serialize());
			std::cout << u8"[������ ������ JSON ��ü]\n" << json_utf8 << "\n";

			std::cout << u8"\n[�ó����� ����]\n";
			std::cout << u8"�ĺ���: " << to_utf8(scenario_json.at(U("scenario_id")).as_string()) << "\n";
			std::cout << u8"����: " << to_utf8(scenario_json.at(U("scenario_title")).as_string()) << "\n";
			std::cout << u8"�װ��� ����: " << scenario_json.at(U("aircraft_count")).as_integer() << "\n";

			auto battery = scenario_json.at(U("battery_location")).as_object();
			std::cout << u8"\n[���� ��ġ]\n";
			std::cout << u8"����: " << battery.at(U("latitude")).as_double() << "\n";
			std::cout << u8"�浵: " << battery.at(U("longitude")).as_double() << "\n";
			std::cout << u8"��: " << battery.at(U("altitude")).as_double() << "\n";

			std::cout << u8"\n[�װ��� ���]\n";
			auto aircraft_array = scenario_json.at(U("aircraft_list")).as_array();
			for (size_t i = 0; i < aircraft_array.size(); ++i) {
				auto aircraft = aircraft_array[i].as_object();
				std::cout << u8"\n�װ��� " << i + 1 << "\n";
				std::cout << u8"�ĺ���: " << to_utf8(aircraft.at(U("aircraft_id")).as_string()) << "\n";
				std::cout << u8"�Ǿ�����: " << to_utf8(aircraft.at(U("friend_or_foe")).as_string()) << "\n";

				auto start = aircraft.at(U("start_point")).as_object();
				std::cout << u8"[����]\n";
				std::cout << u8"����: " << start.at(U("latitude")).as_double() << "\n";
				std::cout << u8"�浵: " << start.at(U("longitude")).as_double() << "\n";
				std::cout << u8"��: " << start.at(U("altitude")).as_double() << "\n";

				auto end = aircraft.at(U("end_point")).as_object();
				std::cout << u8"[����]\n";
				std::cout << u8"����: " << end.at(U("latitude")).as_double() << "\n";
				std::cout << u8"�浵: " << end.at(U("longitude")).as_double() << "\n";
				std::cout << u8"��: " << end.at(U("altitude")).as_double() << "\n";
			}
		}
		else {
			std::cerr << u8"�ó����� ��û ����: HTTP " << response.status_code() << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << u8"���� �߻�: " << e.what() << std::endl;
	}
}
