#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "StartSignalReceiver.h"
#include "GlobalState.h"
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

std::string g_scenario_id;  // GlobalState.h 변수 여기에 정의

void setup_start_signal_listener(
    const std::string& address,
    const std::string& client_id,
    std::function<void()> on_start_callback)
{
    static http_listener listener(utility::conversions::to_string_t(address));

    listener.support(methods::POST, [=](http_request request) {
        request.extract_json().then([=](json::value body) {
            auto cmd = utility::conversions::to_utf8string(body[U("command")].as_string());

            if (cmd == "start") {
                // scenario_id 파싱 추가
                if (body.has_field(U("scenario_id"))) {
                    g_scenario_id = utility::conversions::to_utf8string(body[U("scenario_id")].as_string());
                    std::cout << u8"[" << client_id << u8"] 시작 신호 수신! 시나리오 ID: " << g_scenario_id << std::endl;
                }
                else {
                    std::cerr << u8"[" << client_id << u8"] 오류: scenario_id가 포함되지 않았습니다.\n";
                }

                // 콜백 실행 (예: request_scenario 호출)
                on_start_callback();
            }
        }).wait();

        request.reply(status_codes::OK, U("OK"));
    });

    try {
        listener.open().wait();
        std::cout << u8"[" << client_id << u8"] /start 대기 중 at " << address << "\n";
    }
    catch (std::exception& e) {
        std::cerr << u8"[" << client_id << u8"] 리스너 시작 실패: " << e.what() << std::endl;
    }
}