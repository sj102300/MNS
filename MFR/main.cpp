#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "StartSignalReceiver.h"
#include "ScenarioManager.h"
#include "ScenarioInfoPrinter.h"

#include <windows.h>
#include <iostream>
#include <memory>  // unique_ptr

// 설정 상수
constexpr char HTTP_LISTEN_ADDRESS[] = "http://127.0.0.1:9015";
constexpr char CLIENT_ID[] = "MFR";
constexpr char SCENARIO_SERVER_URL[] = "http://127.0.0.1:8000";  // 시나리오 서버 주소

int main() {
    SetConsoleOutputCP(CP_UTF8);  // 콘솔 UTF-8 설정

    // ScenarioManager 객체를 미리 생성
    auto scenario_manager = std::make_shared<ScenarioManager>(SCENARIO_SERVER_URL);
    ScenarioInfoPrinter printer;

    setup_start_signal_listener(
        HTTP_LISTEN_ADDRESS,
        CLIENT_ID,
        [scenario_manager, &printer](const std::string& scenario_id) {
            std::cout << u8"[" << CLIENT_ID << u8"] 시나리오 요청 시작\n";
            if (!scenario_manager->requestScenario(scenario_id)) {
                std::cerr << u8"[" << CLIENT_ID << u8"] 시나리오 요청 실패\n";
            }
            // 여기서 출력
            printer.printInfo(*scenario_manager);
            printer.printBattery(*scenario_manager);
            printer.printAircraftList(*scenario_manager);
        }
    );

    std::cout << u8"[" << CLIENT_ID << u8"] OCC에서 시작신호를 기다립니다.\n";
    std::cin.get();
    return 0;
}
