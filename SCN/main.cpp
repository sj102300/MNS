#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCN.h"

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

	http_listener post_listener(U("http://127.0.0.1:8000/scenario/detail"));
	post_listener.support(methods::POST, handle_post);

	http_listener get_listener(U("http://127.0.0.1:8000/scenario/list"));
	get_listener.support(methods::GET, handle_get);

	try {
		post_listener.open().wait();
		get_listener.open().wait();
		std::cout << u8"�ó����� ������ ���� ���Դϴ�: POST /scenario/detail" << std::endl;
		std::cout << u8"�ó����� ������ ���� ���Դϴ�: GET /scenario/list" << std::endl;
		std::cout << u8".json �ó����� ������ �ִ� ��ġ���� ����Ǿ�� �մϴ�." << std::endl;

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
