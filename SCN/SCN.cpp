#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCN.h"

#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// UTF-16 → UTF-8 변환 함수 정의
std::string to_utf8(const std::wstring& wstr) {
	if (wstr.empty()) return {};

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
	return result;
}

// POST 요청 처리 함수 정의
void handle_post(http_request request) {
	try {
		std::cout << u8"[파일 열기 시도: Scenario01.json]" << std::endl;

		std::ifstream file("Scenario01.json");
		if (!file.is_open()) {
			std::cerr << u8"파일을 열 수 없습니다." << std::endl;
			request.reply(status_codes::InternalError, U("시나리오 파일 열기 실패"));
			return;
		}

		std::cout << u8"[파일 열기 성공]" << std::endl;

		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		json::value scenario_json;  // JSON 내용
		try {
			scenario_json = json::value::parse(buffer.str());
			std::cout << u8"[JSON 파싱 성공]" << std::endl;

			std::string utf8_json = to_utf8(scenario_json.serialize());
			std::cout << u8"[JSON 응답 내용]\n" << utf8_json << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "JSON 파싱 예외: " << e.what() << std::endl;
			request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
			return;
		}

		request.reply(status_codes::OK, scenario_json);
	}
	catch (const std::exception& e) {
		std::cerr << "전체 예외 발생: " << e.what() << std::endl;
		request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
	}
}
