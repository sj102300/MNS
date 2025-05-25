#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"
#include <cpprest/http_listener.h>
#include <windows.h>
#include <iostream>

using namespace web::http::experimental::listener;
using utility::conversions::to_string_t;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // === 설정 상수 ===
    const std::string scenario_dir = "./Scenarios";
    //const std::string base_address = "http://192.168.15.30:8080";
    const std::string base_address = "http://localhost:8000";

    // === 서비스 초기화 ===
    ScenarioService scenario_service(scenario_dir);
    scenario_service.loadMetaCache();

    http_listener post_info_listener(to_string_t(base_address + "/scenario/info"));
    post_info_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostInfo, &scenario_service, std::placeholders::_1));
    http_listener post_save_listener(to_string_t(base_address + "/scenario/save"));
    post_save_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostSave, &scenario_service, std::placeholders::_1));
    http_listener get_list_listener(to_string_t(base_address + "/scenario/list"));
    get_list_listener.support(web::http::methods::GET, std::bind(&ScenarioService::handleGet, &scenario_service, std::placeholders::_1));

    try {
        try {
            post_info_listener.open().wait();
            get_list_listener.open().wait();
            post_save_listener.open().wait();
            std::cout << u8"[SCN 주소] " << base_address << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << u8"리스너 포트 열기 실패: " << e.what() << std::endl;
        }

        std::cout << u8"[서버 실행 중] POST /scenario/info" << std::endl;
        std::cout << u8"[서버 실행 중] POST /scenario/save" << std::endl;
        std::cout << u8"[서버 실행 중] GET /scenario/list" << std::endl;

        std::string dummy;
        std::getline(std::cin, dummy);
    }
    catch (const std::exception& e) {
        std::cerr << u8"서버 예외: " << e.what() << std::endl;
    }

    // 향후 종료 기능 구현 예정
    //post_info_listener.close().wait();
    //post_save_listener.close().wait();
    //get_list_listener.close().wait();

}

