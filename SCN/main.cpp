#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCN.h"

#include <windows.h>
#include <iostream>
#include <filesystem>
#include <cpprest/http_listener.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// ===  전역 상수 정의 === //
const std::string SCENARIO_DIR = "./Scenarios";
const utility::string_t API_SCENARIO_DETAIL = U("http://127.0.0.1:8000/scenario/detail");
const utility::string_t API_SCENARIO_LIST = U("http://127.0.0.1:8000/scenario/list");

int main() {
	SetConsoleOutputCP(CP_UTF8);  // 콘솔 출력 인코딩을 UTF-8로 설정

	std::cout << u8"[서버 실행 경로]: " << std::filesystem::current_path().string() << std::endl;

	// 시나리오 캐싱
	load_scenario_meta_cache(SCENARIO_DIR);

	// HTTP 리스너 설정
	http_listener post_listener(API_SCENARIO_DETAIL);
	post_listener.support(methods::POST, handle_post);

	http_listener get_listener(API_SCENARIO_LIST);
	get_listener.support(methods::GET, handle_get);

	try {
		post_listener.open().wait();
		get_listener.open().wait();
		std::cout << u8"시나리오 서버가 실행 중입니다: POST " << utility::conversions::to_utf8string(API_SCENARIO_DETAIL) << std::endl;
		std::cout << u8"시나리오 서버가 실행 중입니다: GET  " << utility::conversions::to_utf8string(API_SCENARIO_LIST) << std::endl;

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
