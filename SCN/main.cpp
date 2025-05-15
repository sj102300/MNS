#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"
#include <cpprest/http_listener.h>
#include <windows.h>
#include <iostream>

using namespace web::http::experimental::listener;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    const std::string scenario_dir = "./Scenarios";  // ��� ��δ� ���⼭ ����

    ScenarioService scenario_service(scenario_dir);
    scenario_service.loadMetaCache();

    http_listener post_info_listener(U("http://127.0.0.1:8000/scenario/info"));
    post_info_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostInfo, &scenario_service, std::placeholders::_1));
    http_listener post_save_listener(U("http://127.0.0.1:8000/scenario/save"));
    post_save_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostSave, &scenario_service, std::placeholders::_1));
    http_listener get_list_listener(U("http://127.0.0.1:8000/scenario/list"));
    get_list_listener.support(web::http::methods::GET, std::bind(&ScenarioService::handleGet, &scenario_service, std::placeholders::_1));

    try {
        post_info_listener.open().wait();
        get_list_listener.open().wait();
        post_save_listener.open().wait();

        std::cout << u8"[���� ���� ��] POST /scenario/info" << std::endl;
        std::cout << u8"[���� ���� ��] POST /scenario/save" << std::endl;
        std::cout << u8"[���� ���� ��] GET /scenario/list" << std::endl;

        std::string dummy;
        std::getline(std::cin, dummy);
    }
    catch (const std::exception& e) {
        std::cerr << "���� ����: " << e.what() << std::endl;
    }

    // ���� ���� ��� ���� ����
    //post_info_listener.close().wait();
    //post_save_listener.close().wait();
    //get_list_listener.close().wait();

}

