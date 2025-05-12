#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "StartSignalReceiver.h"
#include "MFR.h"  // request_scenario()

#include <windows.h>
#include <iostream>

// 설정 상수
constexpr char ADDRESS[] = "http://localhost:9015";
constexpr char CLIENT_ID[] = "MFR";

int main() {
	SetConsoleOutputCP(CP_UTF8);  // 콘솔을 UTF-8 출력으로 설정
	

    setup_start_signal_listener(
        ADDRESS,    // MFR의 포트
        CLIENT_ID,  // 클라이언트 식별자
        []() {
            request_scenario();  // 시작 신호 수신 시 실행할 동작
        }
    );

    std::cout << u8"[" << CLIENT_ID << u8"] OCC에서 시작신호를 기다립니다.\n";
	std::cin.get();
	return 0;
}
