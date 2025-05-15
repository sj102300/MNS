#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"
#include <cpprest/http_listener.h>
#include <windows.h>
#include <iostream>

using namespace web::http::experimental::listener;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    const std::string scenario_dir = "./Scenarios";  // 모든 경로는 여기서 관리

    ScenarioService scenario_service(scenario_dir);
    scenario_service.loadMetaCache();

    http_listener post_listener(U("http://127.0.0.1:8000/scenario/detail"));
    post_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePost, &scenario_service, std::placeholders::_1));

    http_listener get_listener(U("http://127.0.0.1:8000/scenario/list"));
    get_listener.support(web::http::methods::GET, std::bind(&ScenarioService::handleGet, &scenario_service, std::placeholders::_1));

    try {
        post_listener.open().wait();
        get_listener.open().wait();
        std::cout << u8"[서버 실행 중] POST /scenario/detail, GET /scenario/list" << std::endl;

        std::string dummy;
        std::getline(std::cin, dummy);
    }
    catch (const std::exception& e) {
        std::cerr << "서버 예외: " << e.what() << std::endl;
    }
}

