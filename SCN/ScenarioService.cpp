#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"
#include "Utils.h"  // to_utf8 함수 포함

#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

ScenarioService::ScenarioService() {
    cached_json_response_ = json::value::array();
}

void ScenarioService::loadMetaCache(const std::string& directory) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cached_json_response_ = json::value::array();

    size_t index = 0;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            try {
                std::ifstream file(entry.path());
                if (!file.is_open()) continue;

                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();

                auto data = json::value::parse(buffer.str());

                if (data.has_field(U("scenario_id")) && data.has_field(U("scenario_title"))) {
                    json::value item;
                    item[U("scenario_id")] = data[U("scenario_id")];
                    item[U("scenario_title")] = data[U("scenario_title")];
                    cached_json_response_[index++] = item;
                }
            }
            catch (...) {
                std::cerr << u8"[오류] 캐싱 중 JSON 파싱 실패: " << entry.path().string() << std::endl;
            }
        }
    }

    std::cout << u8"[SCN] 시나리오 캐시 로드 완료 (총 " << index << u8"개)\n";
}

void ScenarioService::handleGet(http_request request) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    std::cout << u8"[GET /scenario/list] 반환 시나리오 수: " << cached_json_response_.size() << std::endl;
    request.reply(status_codes::OK, cached_json_response_);
}

void ScenarioService::handlePost(http_request request) {
    request.extract_json().then([=](pplx::task<json::value> task) {
        try {
            auto body = task.get();
            if (!body.has_field(U("scenario_id"))) {
                request.reply(status_codes::BadRequest, U("Missing scenario_id"));
                return;
            }

            auto scenario_id = utility::conversions::to_utf8string(body.at(U("scenario_id")).as_string());
            std::string filename = scenario_dir_ + "/" + scenario_id + ".json";

            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << u8"[오류] 파일 열기 실패: " << filename << std::endl;
                request.reply(status_codes::NotFound, U("Scenario file not found"));
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            json::value scenario_data = json::value::parse(buffer.str());
            std::cout << u8"[시나리오 요청 ID]: " << scenario_id << std::endl;
            std::cout << u8"[JSON 파싱 성공]\n";

            std::string utf8_json = utils::to_utf8(scenario_data.serialize());
            std::cout << u8"[응답 내용]\n" << utf8_json << std::endl;

            request.reply(status_codes::OK, scenario_data);
        }
        catch (const std::exception& e) {
            std::cerr << u8"[예외] handlePost: " << e.what() << std::endl;
            request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
        }
        }).wait();
}
