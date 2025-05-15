#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "StartSignalReceiver.h"
#include "ScenarioManager.h"
#include "ScenarioInfoPrinter.h"

#include <windows.h>
#include <iostream>
#include <memory>  // unique_ptr

// ���� ���
constexpr char HTTP_LISTEN_ADDRESS[] = "http://127.0.0.1:9015";
constexpr char CLIENT_ID[] = "MFR";
constexpr char SCENARIO_SERVER_URL[] = "http://127.0.0.1:8000";  // �ó����� ���� �ּ�

int main() {
    SetConsoleOutputCP(CP_UTF8);  // �ܼ� UTF-8 ����

    // ScenarioManager ��ü�� �̸� ����
    auto scenario_manager = std::make_shared<ScenarioManager>(SCENARIO_SERVER_URL);
    ScenarioInfoPrinter printer;

    setup_start_signal_listener(
        HTTP_LISTEN_ADDRESS,
        CLIENT_ID,
        [scenario_manager, &printer](const std::string& scenario_id) {
            std::cout << u8"[" << CLIENT_ID << u8"] �ó����� ��û ����\n";
            if (!scenario_manager->requestScenario(scenario_id)) {
                std::cerr << u8"[" << CLIENT_ID << u8"] �ó����� ��û ����\n";
            }
            // ���⼭ ���
            printer.printInfo(*scenario_manager);
            printer.printBattery(*scenario_manager);
            printer.printAircraftList(*scenario_manager);
        }
    );

    std::cout << u8"[" << CLIENT_ID << u8"] OCC���� ���۽�ȣ�� ��ٸ��ϴ�.\n";
    std::cin.get();
    return 0;
}
