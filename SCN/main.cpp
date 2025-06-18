#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioService.h"
#include "AsciiBanner.h"
#include <cpprest/http_listener.h>
#include <windows.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <csignal>

using namespace web::http::experimental::listener;
using utility::conversions::to_string_t;

// === ���� ���� ���� ===
std::mutex g_mtx;
std::condition_variable g_cv;
bool g_should_exit = false;

// === Ctrl+C �ڵ鷯 ===
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cout << "\n[���� ��û ����] Ctrl+C" << std::endl;
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            g_should_exit = true;
        }
        g_cv.notify_one();
        return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler(consoleHandler, TRUE);  // Ctrl+C �ڵ鷯 ���

    // === ���� ��� ===
    const std::string scenario_dir = "./Scenarios";
    const std::string base_address = "http://192.168.2.30:8080";

    // === ���� �ʱ�ȭ ===
    ScenarioService scenario_service(scenario_dir);

    http_listener post_info_listener(to_string_t(base_address + "/scenario/info"));
    post_info_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostInfo, &scenario_service, std::placeholders::_1));
    
    http_listener post_save_listener(to_string_t(base_address + "/scenario/save"));
    post_save_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostSave, &scenario_service, std::placeholders::_1));
    
    http_listener get_list_listener(to_string_t(base_address + "/scenario/list"));
    get_list_listener.support(web::http::methods::GET, std::bind(&ScenarioService::handleGet, &scenario_service, std::placeholders::_1));

    try {
        // ������ ����
        post_info_listener.open().wait();
        get_list_listener.open().wait();
        post_save_listener.open().wait();

        banner::printAsciiBanner();
        scenario_service.loadMetaCache();
        std::cout << u8"[SCN �ּ�] " << base_address << std::endl;
        std::cout << u8"[���� ���� ��] POST /scenario/info" << std::endl;
        std::cout << u8"[���� ���� ��] POST /scenario/save" << std::endl;
        std::cout << u8"[���� ���� ��] GET /scenario/list" << std::endl;
        std::cout << u8"[���� ���] Ctrl+C�� ������ ������ �����ϰ� �����մϴ�\n";

        // === ���� ��ȣ���� ��� ===
        std::unique_lock<std::mutex> lock(g_mtx);
        g_cv.wait(lock, [] { return g_should_exit; });

        // === ������ �ݱ� ===
        post_info_listener.close().wait();
        post_save_listener.close().wait();
        get_list_listener.close().wait();
        std::cout << u8"[���� ����] ��� ������ ����" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << u8"[���� ����] " << e.what() << std::endl;
    }

    return 0;
}