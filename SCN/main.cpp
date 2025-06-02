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

// === 전역 종료 제어 ===
std::mutex g_mtx;
std::condition_variable g_cv;
bool g_should_exit = false;

// === Ctrl+C 핸들러 ===
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cout << "\n[종료 요청 수신] Ctrl+C" << std::endl;
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
    SetConsoleCtrlHandler(consoleHandler, TRUE);  // Ctrl+C 핸들러 등록

    // === 설정 상수 ===
    const std::string scenario_dir = "./Scenarios";
    const std::string base_address = "http://192.168.2.30:8080";

    // === 서비스 초기화 ===
    ScenarioService scenario_service(scenario_dir);

    http_listener post_info_listener(to_string_t(base_address + "/scenario/info"));
    post_info_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostInfo, &scenario_service, std::placeholders::_1));
    
    http_listener post_save_listener(to_string_t(base_address + "/scenario/save"));
    post_save_listener.support(web::http::methods::POST, std::bind(&ScenarioService::handlePostSave, &scenario_service, std::placeholders::_1));
    
    http_listener get_list_listener(to_string_t(base_address + "/scenario/list"));
    get_list_listener.support(web::http::methods::GET, std::bind(&ScenarioService::handleGet, &scenario_service, std::placeholders::_1));

    try {
        // 리스너 열기
        post_info_listener.open().wait();
        get_list_listener.open().wait();
        post_save_listener.open().wait();

        banner::printAsciiBanner();
        scenario_service.loadMetaCache();
        std::cout << u8"[SCN 주소] " << base_address << std::endl;
        std::cout << u8"[서버 실행 중] POST /scenario/info" << std::endl;
        std::cout << u8"[서버 실행 중] POST /scenario/save" << std::endl;
        std::cout << u8"[서버 실행 중] GET /scenario/list" << std::endl;
        std::cout << u8"[종료 방법] Ctrl+C를 누르면 서버를 안전하게 종료합니다\n";

        // === 종료 신호까지 대기 ===
        std::unique_lock<std::mutex> lock(g_mtx);
        g_cv.wait(lock, [] { return g_should_exit; });

        // === 리스너 닫기 ===
        post_info_listener.close().wait();
        post_save_listener.close().wait();
        get_list_listener.close().wait();
        std::cout << u8"[정상 종료] 모든 리스너 닫힘" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << u8"[서버 예외] " << e.what() << std::endl;
    }

    return 0;
}