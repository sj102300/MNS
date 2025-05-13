#pragma once

#include <string>

// UTF-16 → UTF-8 변환
std::string to_utf8(const std::wstring& wstr);

// 서버에서 시나리오 요청 및 출력
void request_scenario();


