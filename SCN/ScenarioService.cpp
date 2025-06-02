#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"

#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <set>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

ScenarioService::ScenarioService(const std::string& scenario_dir)
    : scenario_dir_(scenario_dir) {
    std::filesystem::create_directories(scenario_dir_);  // create directory
    cached_json_response_ = web::json::value::array();
}

void ScenarioService::loadMetaCache() { 
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cached_json_response_ = web::json::value::array();

    if (!std::filesystem::is_directory(scenario_dir_)) {
        std::cerr << u8"[오류] 경로가 디렉터리가 아님: " << scenario_dir_ << std::endl;
        return;
    }

    size_t index = 0;
    for (const auto& entry : std::filesystem::directory_iterator(scenario_dir_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            try {
                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    std::cerr << u8"[경고] 파일 열기 실패: " << entry.path() << std::endl;
                    continue;
                }

                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();

                auto data = web::json::value::parse(buffer.str());

                if (data.has_field(U("scenario_id")) && data.has_field(U("scenario_title"))) {
                    web::json::value item;
                    item[U("scenario_id")] = data[U("scenario_id")];
                    item[U("scenario_title")] = data[U("scenario_title")];
                    cached_json_response_[index++] = item;
                }
                else {
                    std::cerr << u8"[경고] 필드 누락: " << entry.path() << std::endl;
                }
            }
            catch (...) {
                std::cerr << u8"[오류] JSON 파싱 실패: " << entry.path() << std::endl;
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

void ScenarioService::handlePostInfo(http_request request) {
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

            std::string utf8_json = utility::conversions::to_utf8string(scenario_data.serialize());
            std::cout << u8"[응답 내용]\n" << utf8_json << std::endl;

            request.reply(status_codes::OK, scenario_data);
        }
        catch (const std::exception& e) {
            std::cerr << u8"[예외] handlePost: " << e.what() << std::endl;
            request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
        }
        }).wait();
}

void ScenarioService::handlePostSave(http_request request) {
    request.extract_json().then([=](pplx::task<json::value> task) {
        try {
            auto body = task.get();

            // generate new ID
            std::string new_id = generateNextScenarioId();
            if (new_id.empty()) {
                std::cerr << u8"[오류] 시나리오 ID 한도 초과 (99개 초과 저장 시도)\n";
                request.reply(status_codes::BadRequest, U("시나리오 ID 한도 초과 (99개 초과 저장 시도)"));
                return;
            }
            body[U("scenario_id")] = json::value::string(utility::conversions::to_string_t(new_id));
            
            // save directory and filename
            std::string filename = scenario_dir_ + "/" + new_id + ".json";

            // save
            std::ofstream file(filename);
            if (!file.is_open()) {
                std::cerr << u8"[오류] 파일 저장 실패: " << filename << std::endl;
                request.reply(status_codes::InternalError, U("파일 저장 실패"));
                return;
            }

            file << utility::conversions::to_utf8string(body.serialize());
            file.close();

            // reload
            loadMetaCache();
            std::cout << u8"[SCN] 시나리오 저장 완료: " << filename << std::endl;

            request.reply(status_codes::OK, U("시나리오 저장 완료"));
        }
        catch (const std::exception& e) {
            std::cerr << u8"[예외] handlePostSave: " << e.what() << std::endl;
            request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
        }
    }).wait();
}

// Generates a new scenario ID by finding the smallest unused number in "SCENE-XX.json" format.
std::string ScenarioService::generateNextScenarioId() {
    std::set<int> used_numbers;  // Set of already used scenario numbers
    std::regex pattern(R"(SCENE-(\d{2})\.json)");  // Regex pattern to match file name like SCENE-01.json

    for (const auto& entry : std::filesystem::directory_iterator(scenario_dir_)) {
        if (entry.is_regular_file()) {
            std::smatch match;
            std::string filename = entry.path().filename().string();

            // If the filename matches the pattern, extract the number
            if (std::regex_match(filename, match, pattern)) {
                int num = std::stoi(match[1]);  // Convert "03" to 3
                used_numbers.insert(num);       // Mark this number as used
            }
        }
    }

    // Find the smallest unused number starting from 1
    int new_num = 1;
    while (used_numbers.count(new_num)) {
        ++new_num;
    }

    // LIMIT: maximum = 99
    if (new_num > 99) {
        return "";  // ID generation failed
    }

    // Format the new ID as "SCENE-XX"
    std::ostringstream oss;
    oss << "SCENE-" << std::setw(2) << std::setfill('0') << new_num;
    return oss.str();  // 예: SCENE-03
}