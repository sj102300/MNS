#pragma once

#include <cpprest/http_listener.h>
#include <string>

// UTF-16 �� UTF-8 ��ȯ �Լ� ����
std::string to_utf8(const std::wstring& wstr);

// POST ��û ó�� �Լ� ����
void handle_post(web::http::http_request request);

// GET ��û ó�� �Լ� ����
void handle_get(web::http::http_request request);

// �ó����� ��� ĳ�� �Լ�
void load_scenario_meta_cache(const std::string& directory_path);