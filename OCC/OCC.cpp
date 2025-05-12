#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::client;

// 시작 신호 전송 함수
void send_start_signal(const std::string& target_url, const std::string& client_id) {
    http_client client(utility::conversions::to_string_t(target_url));

    json::value postData;
    postData[U("command")] = json::value::string(U("start"));
    postData[U("client_id")] = json::value::string(utility::conversions::to_string_t(client_id));

    try {
        client.request(methods::POST, U("/start"), postData)
            .then([=](http_response response) {
            if (response.status_code() == status_codes::OK) {
                std::wcout << u8"[OCC] " << utility::conversions::to_string_t(client_id)
                    << u8" → 시작신호 전송 성공\n";
            }
            else {
                std::wcout << u8"[OCC] " << utility::conversions::to_string_t(client_id)
                    << u8" → 응답 오류: " << response.status_code() << std::endl;
            }
                }).wait();
    }
    catch (const std::exception& e) {
        std::cerr << u8"[OCC] 예외 발생: " << e.what() << std::endl;
    }
}
