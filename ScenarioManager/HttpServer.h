#pragma once

#include <string>
#include <functional>
#include <cpprest/http_listener.h>

namespace sm {
    class HttpServer {
    public:
        HttpServer(const std::string& address, const std::string& client_id);
        ~HttpServer();

        void setOnStartCallback(std::function<void(const std::string&)> cb);
        void setOnQuitCallback(std::function<void()> cb);

        bool start();
        void stop();

    private:
        web::http::experimental::listener::http_listener listener_;
        std::string client_id_;
        std::function<void(const std::string&)> on_start_callback_;
        std::function<void()> on_quit_callback_;
        bool is_open_ = false;
    };
}