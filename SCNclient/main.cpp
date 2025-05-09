#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCNclient.h"

#include <windows.h>
#include <iostream>

int main() {
	SetConsoleOutputCP(CP_UTF8);  // 콘솔을 UTF-8 출력으로 설정
	
	request_scenario();

	std::cout << u8"\n프로그램 종료... 아무 키나 누르세요.\n";
	std::cin.get();
	return 0;
}
