#pragma once

#include <cpprest/http_listener.h>
#include <string>

// UTF-16 → UTF-8 변환 함수 선언
std::string to_utf8(const std::wstring& wstr);

// POST 요청 처리 함수 선언
void handle_post(web::http::http_request request);

// GET 요청 처리 함수 선언
void handle_get(web::http::http_request request);

// 시나리오 목록 캐싱 함수
void load_scenario_meta_cache(const std::string& directory_path);