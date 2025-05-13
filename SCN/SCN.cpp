#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCN.h"

#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>
#include <filesystem>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// 시나리오 파일들이 있는 폴더 경로 (로컬 고정 경로)
const std::string FOLDER_PATH = "C:\\Users\\user\\source\\repos\\MnS_LSY\\x64\\Debug\\Scenarios";

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
    request.extract_json().then([request](pplx::task<json::value> task) {
        try {
            auto body = task.get();

            // "scenario_id" 필드가 있는지 확인
            if (!body.has_field(U("scenario_id"))) {
                request.reply(status_codes::BadRequest, U("Missing scenario_id"));
                return;
            }

            auto scenario_id = utility::conversions::to_utf8string(body.at(U("scenario_id")).as_string());

            // 예: "SCENE-03" → "SCENE-03.json"로 파일명 구성
            std::string filename = "Scenarios\\" + scenario_id + ".json";
            std::cout << u8"[시나리오 요청 ID]: " << scenario_id << std::endl;
            std::cout << u8"[파일 열기 시도]: " << filename << std::endl;

            // 파일 열기 시도
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << u8"[오류] 파일 열기 실패: " << filename << std::endl;
                request.reply(status_codes::NotFound, U("Scenario file not found"));
                return;
            }

            // 파일 내용 읽기
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            json::value scenario_data = json::value::parse(buffer.str());
            std::cout << u8"[JSON 파싱 성공]" << std::endl;

            std::string utf8_json = to_utf8(scenario_data.serialize());
            std::cout << u8"[응답 내용]\n" << utf8_json << std::endl;

            request.reply(status_codes::OK, scenario_data);
        }
        catch (const std::exception& e) {
            std::cerr << u8"[예외] handle_post: " << e.what() << std::endl;
            request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
        }
        }
	).wait(); // 비동기 작업 대기
}

void handle_get(http_request request) {
	std::vector<json::value> scenario_list;

	for (const auto& entry : std::filesystem::directory_iterator(FOLDER_PATH)) {
		if (entry.is_regular_file() && entry.path().extension() == ".json") {
			try {
				std::ifstream file(entry.path());
				if (!file.is_open()) continue;

				std::stringstream buffer;
				buffer << file.rdbuf();
				file.close();

				json::value data = json::value::parse(buffer.str());

				if (data.has_field(U("scenario_id")) && data.has_field(U("scenario_title"))) {
					json::value item;
					item[U("scenario_id")] = data[U("scenario_id")];
					item[U("scenario_title")] = data[U("scenario_title")];
					scenario_list.push_back(item);
				}
			}
			catch (...) {
				std::cerr << u8"[오류] JSON 파싱 실패: " << entry.path().string() << std::endl;
				continue;
			}
		}
	}

	// 응답 배열 생성
	json::value response = json::value::array();
	for (size_t i = 0; i < scenario_list.size(); ++i) {
		response[i] = scenario_list[i];
	}

	std::cout << u8"[GET /scenarios] 반환된 시나리오 수: " << scenario_list.size() << std::endl;
	request.reply(status_codes::OK, response);
}

