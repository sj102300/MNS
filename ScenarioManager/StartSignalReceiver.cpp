#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "StartSignalReceiver.h"
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

void setup_start_signal_listener(
    const std::string& address,
    const std::string& client_id,
    std::function<void(const std::string&)> on_start_callback
    //std::function<void(const std::string&)> on_quit_callback
)
{
    static http_listener listener(utility::conversions::to_string_t(address));
    listener.support(methods::POST, [=](http_request request) {
        request.extract_json().then([=](json::value body) {
            if (body.has_field(U("command")) && body.has_field(U("scenario_id"))) {
                auto cmd = utility::conversions::to_utf8string(body[U("command")].as_string());
                auto scenario_id = utility::conversions::to_utf8string(body[U("scenario_id")].as_string());

                if (cmd == "start") {
                    std::cout << u8"[" << client_id << u8"] 시작 신호 수신! 시나리오 ID: " << scenario_id << "\n";
                    on_start_callback(scenario_id);
                }
            }
            }).wait();

        request.reply(status_codes::OK, U("OK"));
        });

    try {
        listener.open().wait();
        std::cout << u8"[" << client_id << u8"] /start 대기 중 at " << address << "\n";
    }
    catch (std::exception& e) {
        std::cerr << u8"[" << client_id << u8"] 리스너 시작 실패: " << e.what() << "\n";
    }
}
