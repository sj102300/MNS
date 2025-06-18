#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "HttpServer.h"
#include "AsciiBanner.h"
#include <cpprest/json.h>
#include <iostream>
#include <thread>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

namespace sm {

    HttpServer::HttpServer(const std::string& address, const std::string& client_id)
        : listener_(utility::conversions::to_string_t(address)), client_id_(client_id)
    {
        listener_.support(methods::POST, [=](http_request request) {
            request.extract_json().then([=](json::value body) {
                try {
                    if (body.has_field(U("command"))) {
                        auto cmd = utility::conversions::to_utf8string(body[U("command")].as_string());
                        auto scenario_id = utility::conversions::to_utf8string(body[U("scenario_id")].as_string());

                        if (cmd == "start" && on_start_callback_) {
                            std::cout << u8"[" << client_id_ << u8"] 시작 신호 수신! 시나리오 ID: " << scenario_id << "\n";
                            on_start_callback_(scenario_id);
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << u8"[HttpServer] POST 파싱 오류: " << e.what() << "\n";
                }
                }).wait();

            request.reply(status_codes::OK, U("START_ACK"));
            });

        listener_.support(methods::GET, [=](http_request request) {
            try {
                auto path = uri::decode(request.relative_uri().path());

                if (path == U("/quit")) {
                    std::cout << u8"[" << client_id_ << u8"] 종료 신호 수신 (GET /quit)\n";
                    request.reply(status_codes::OK, U("QUIT_ACK"));

                    if (on_quit_callback_) on_quit_callback_();         //handleQuitSignal();
                }
                else {
                    request.reply(status_codes::NotFound, U("Unknown path"));
                }
            }
            catch (const std::exception& e) {
                std::cerr << u8"[HttpServer] GET 핸들러 오류: " << e.what() << "\n";
                request.reply(status_codes::InternalError, U("GET handler error"));
            }
            });
    }

    HttpServer::~HttpServer() {
        stop();
    }

    void HttpServer::setOnStartCallback(std::function<void(const std::string&)> cb) {
        on_start_callback_ = std::move(cb);
    }

    void HttpServer::setOnQuitCallback(std::function<void()> cb) {
        on_quit_callback_ = std::move(cb);      //handleQuitSignal();
    }

    bool HttpServer::start() {
        if (is_open_) {
            std::cout << u8"[" << client_id_ << u8"] 리스너는 이미 열려 있습니다.\n";
            return false;
        }

        const int max_retries = 10;
        const std::chrono::seconds retry_interval(1);
        std::string retry_dots;

        for (int attempt = 1; attempt <= max_retries; ++attempt) {
            try {
                listener_.open().wait();
                is_open_ = true;
                banner::printAsciiBanner();

                std::cout << u8"\n[" << client_id_ << u8"] 리스너 시작됨. 대기 중...\n";
                return true;
            }
            catch (const std::exception& e) {
                retry_dots += ".";

                std::cerr << u8"\r[" << client_id_ << u8"] 리스너 시작 재시도 중" << retry_dots << std::flush;
                if (attempt < max_retries) {
                    std::this_thread::sleep_for(retry_interval);
                }
                else {
                    std::cerr << u8"\n[" << client_id_ << u8"] 리스너 재시작 실패: " << e.what() << std::endl;
                }
            }
        }

        return false;
    }

    void HttpServer::stop() {
        if (!is_open_) return;

        try {
            listener_.close().wait();
            is_open_ = false;
            std::cout << u8"[" << client_id_ << u8"] 리스너 정상 종료됨.\n";
        }
        catch (const std::exception& e) {
            std::cerr << u8"[" << client_id_ << u8"] 리스너 종료 실패: " << e.what() << "\n";
        }
    }

}
