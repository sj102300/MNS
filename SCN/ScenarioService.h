#pragma once

#include <cpprest/http_listener.h>
#include <mutex>
#include <string>
#include <vector>

class ScenarioService {
public:
    explicit ScenarioService(const std::string& scenario_dir);  // 생성자 인자로 받음

    void loadMetaCache();
    void handleGet(web::http::http_request request);
    void handlePost(web::http::http_request request);

private:
    web::json::value cached_json_response_;
    std::mutex cache_mutex_;

    std::string scenario_dir_;
};

