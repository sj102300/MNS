#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCNserver.h"

#include <windows.h>
#include <iostream>
#include <filesystem>
#include <cpprest/http_listener.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

int main() {
	SetConsoleOutputCP(CP_UTF8);  // �ܼ� ��� ���ڵ��� UTF-8�� ����

	std::cout << u8"[���� ���� ���]: " << std::filesystem::current_path().string() << std::endl;

	http_listener listener(U("http://127.0.0.1:8000/scenario/detail"));
	listener.support(methods::POST, handle_post);

	try {
		listener.open().wait();
		std::cout << u8"�ó����� ������ ���� ���Դϴ�: POST /scenario/detail" << std::endl;
		std::cout << u8"Scenario01.json ������ �ִ� ��ġ���� ����Ǿ�� �մϴ�." << std::endl;

		std::string line;
		std::getline(std::cin, line);  // ���� �Է� �� ����
	}
	catch (const std::exception& e) {
		std::cerr << "���� ���� �� ���� �߻�: " << e.what() << std::endl;
	}

	std::cout << u8"���α׷� ����... �ƹ�Ű�� ������ â�ݱ�" << std::endl;
	std::cin.get();

	return 0;
}
