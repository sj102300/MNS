#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <mutex>
#include <string>

class ScenarioService {
public:
    explicit ScenarioService(const std::string& scenario_dir);  // ������ ���ڷ� ����

    void loadMetaCache();
    void handleGet(web::http::http_request request);
    void handlePostInfo(web::http::http_request request);
    void handlePostSave(web::http::http_request request);

private:
    web::json::value cached_json_response_;
    std::mutex cache_mutex_;
    std::string scenario_dir_;
    std::string generateNextScenarioId();
};

