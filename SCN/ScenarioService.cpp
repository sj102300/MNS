#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"
#include "Utils.h"  // to_utf8 �Լ� ����

#include <cpprest/json.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

ScenarioService::ScenarioService(const std::string& scenario_dir)
    : scenario_dir_(scenario_dir) {
    cached_json_response_ = web::json::value::array();
}

void ScenarioService::loadMetaCache() {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cached_json_response_ = web::json::value::array();

    // ���͸� ���� ���� Ȯ��
    if (!std::filesystem::exists(scenario_dir_)) {
        std::cerr << u8"[����] �ó����� ���͸� ����: " << scenario_dir_ << std::endl;
        return;
    }

    if (!std::filesystem::is_directory(scenario_dir_)) {
        std::cerr << u8"[����] ��ΰ� ���͸��� �ƴ�: " << scenario_dir_ << std::endl;
        return;
    }

    size_t index = 0;
    for (const auto& entry : std::filesystem::directory_iterator(scenario_dir_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            try {
                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    std::cerr << u8"[���] ���� ���� ����: " << entry.path() << std::endl;
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
                    std::cerr << u8"[���] �ʵ� ����: " << entry.path() << std::endl;
                }
            }
            catch (...) {
                std::cerr << u8"[����] JSON �Ľ� ����: " << entry.path() << std::endl;
            }
        }
    }

    std::cout << u8"[SCN] �ó����� ĳ�� �ε� �Ϸ� (�� " << index << u8"��)\n";
}

void ScenarioService::handleGet(http_request request) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    std::cout << u8"[GET /scenario/list] ��ȯ �ó����� ��: " << cached_json_response_.size() << std::endl;
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
                std::cerr << u8"[����] ���� ���� ����: " << filename << std::endl;
                request.reply(status_codes::NotFound, U("Scenario file not found"));
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            json::value scenario_data = json::value::parse(buffer.str());
            std::cout << u8"[�ó����� ��û ID]: " << scenario_id << std::endl;
            std::cout << u8"[JSON �Ľ� ����]\n";

            std::string utf8_json = utils::to_utf8(scenario_data.serialize());
            std::cout << u8"[���� ����]\n" << utf8_json << std::endl;

            request.reply(status_codes::OK, scenario_data);
        }
        catch (const std::exception& e) {
            std::cerr << u8"[����] handlePost: " << e.what() << std::endl;
            request.reply(status_codes::InternalError, utility::conversions::to_string_t(e.what()));
        }
        }).wait();
}
