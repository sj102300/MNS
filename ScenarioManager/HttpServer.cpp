#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "HttpServer.h"
#include <cpprest/json.h>
#include <iostream>

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
                            std::cout << u8"[" << client_id_ << u8"] ���� ��ȣ ����! �ó����� ID: " << scenario_id << "\n";
                            on_start_callback_(scenario_id);
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << u8"[HttpServer] POST �Ľ� ����: " << e.what() << "\n";
                }
                }).wait();

            request.reply(status_codes::OK, U("START_ACK"));
            });

        listener_.support(methods::GET, [=](http_request request) {
            try {
                auto path = uri::decode(request.relative_uri().path());

                if (path == U("/quit")) {
                    std::cout << u8"[" << client_id_ << u8"] ���� ��ȣ ���� (GET /quit)\n";
                    if (on_quit_callback_) on_quit_callback_();
                    request.reply(status_codes::OK, U("QUIT_ACK"));
                }
                else {
                    request.reply(status_codes::NotFound, U("Unknown path"));
                }
            }
            catch (const std::exception& e) {
                std::cerr << u8"[HttpServer] GET �ڵ鷯 ����: " << e.what() << "\n";
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
        on_quit_callback_ = std::move(cb);
    }

    bool HttpServer::start() {
        if (is_open_) {
            std::cout << u8"[" << client_id_ << u8"] �����ʴ� �̹� ���� �ֽ��ϴ�.\n";
            return false;
        }

        try {
            listener_.open().wait();
            is_open_ = true;
            std::cout << u8"[" << client_id_ << u8"] ������ ���۵�. ��� ��...\n";
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << u8"[" << client_id_ << u8"] ������ ���� ����: " << e.what() << "\n";
            return false;
        }
    }

    void HttpServer::stop() {
        if (!is_open_) return;

        try {
            listener_.close().wait();
            is_open_ = false;
            std::cout << u8"[" << client_id_ << u8"] ������ ���� �����.\n";
        }
        catch (const std::exception& e) {
            std::cerr << u8"[" << client_id_ << u8"] ������ ���� ����: " << e.what() << "\n";
        }
    }

}