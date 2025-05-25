#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "ScenarioCommandReceiver.h"
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

namespace sm {
    void setup_scenario_command_listener(
        const std::string& address,
        const std::string& client_id,
        std::function<void(const std::string&)> on_start_callback,
        std::function<void()> on_quit_callback
    )
    {
        static http_listener listener(utility::conversions::to_string_t(address));

        // POST 요청 처리: 시작 신호
        listener.support(methods::POST, [=](http_request request) {
            request.extract_json().then([=](json::value body) {
                try {
                    if (body.has_field(U("command"))) {
                        auto cmd = utility::conversions::to_utf8string(body[U("command")].as_string());
                        auto scenario_id = utility::conversions::to_utf8string(body[U("scenario_id")].as_string());

                        if (cmd == "start") {
                            std::cout << u8"[" << client_id << u8"] 시작 신호 수신! 시나리오 ID: " << scenario_id << "\n";
                            on_start_callback(scenario_id);
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << u8"[SCN] POST 파싱 오류: " << e.what() << "\n";
                }
            }).wait();
            
            request.reply(status_codes::OK, U("START_ACK"));
        });

        // GET 요청 처리: 종료 신호
        listener.support(methods::GET, [=](http_request request) {
            try {
                auto path = uri::decode(request.relative_uri().path());

                if (path == U("/quit")) {
                    std::cout << u8"[" << client_id << u8"] 종료 신호 수신 (GET /quit)\n";
                    on_quit_callback();
                    request.reply(status_codes::OK, U("QUIT_ACK"));
                    return;
                }

                // 기타 GET 요청은 무시
                request.reply(status_codes::NotFound, U("Unknown path"));
            }
            catch (const std::exception& e) {
                std::cerr << u8"[SCN] GET 파싱 오류: " << e.what() << "\n";
                request.reply(status_codes::InternalError, U("GET handler error"));
            }
        });

        static bool is_listener_opened = false;

        if (!is_listener_opened) {
            try {
                listener.open().wait();
                is_listener_opened = true;
                std::cout << u8"[" << client_id << u8"] /start 대기 중 at " << address << "\n";
            }
            catch (std::exception& e) {
                std::cerr << u8"[" << client_id << u8"] 리스너 시작 실패: " << e.what() << "\n";
            }
        }
        else {
            std::cout << u8"[" << client_id << u8"] 리스너는 이미 열려 있습니다.\n";
        }
    }
}
