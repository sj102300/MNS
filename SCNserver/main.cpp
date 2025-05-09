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
	SetConsoleOutputCP(CP_UTF8);  // 콘솔 출력 인코딩을 UTF-8로 설정

	std::cout << u8"[서버 실행 경로]: " << std::filesystem::current_path().string() << std::endl;

	http_listener listener(U("http://127.0.0.1:8000/scenario/detail"));
	listener.support(methods::POST, handle_post);

	try {
		listener.open().wait();
		std::cout << u8"시나리오 서버가 실행 중입니다: POST /scenario/detail" << std::endl;
		std::cout << u8"Scenario01.json 파일이 있는 위치에서 실행되어야 합니다." << std::endl;

		std::string line;
		std::getline(std::cin, line);  // 엔터 입력 시 종료
	}
	catch (const std::exception& e) {
		std::cerr << "서버 실행 중 예외 발생: " << e.what() << std::endl;
	}

	std::cout << u8"프로그램 종료... 아무키를 누르면 창닫기" << std::endl;
	std::cin.get();

	return 0;
}
