#pragma once

#include <string>

// UTF-16 �� UTF-8 ��ȯ
std::string to_utf8(const std::wstring& wstr);

// �������� �ó����� ��û �� ���
void request_scenario();


