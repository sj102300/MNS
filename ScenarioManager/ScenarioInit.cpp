#include "ScenarioInit.h"
#include "StartSignalReceiver.h"
#include <windows.h>
#include <iostream>

ScenarioInit::ScenarioInit(const std::string& listen_address,
    const std::string& server_url,
    const std::string& client_id)
    : listen_address_(listen_address), client_id_(client_id),
    scenario_manager_(std::make_shared<ScenarioManager>(server_url)) {
}

void ScenarioInit::run() {
    SetConsoleOutputCP(CP_UTF8);

    setup_start_signal_listener(
        listen_address_,
        client_id_,
        [this](const std::string& scenario_id) {
            handleStartSignal(scenario_id);
        }
    );

    std::cout << u8"[" << client_id_ << u8"] OCC���� ���۽�ȣ�� ��ٸ��ϴ�.\n";
}

void ScenarioInit::handleStartSignal(const std::string& scenario_id) {
    std::cout << u8"[" << client_id_ << u8"] �ó����� ��û ����\n";

    if (!scenario_manager_->requestScenario(scenario_id)) {
        std::cerr << u8"[" << client_id_ << u8"] ��û ����\n";
        return;
    }

    printer_.printInfo(*scenario_manager_);
    printer_.printBattery(*scenario_manager_);
    printer_.printAircraftList(*scenario_manager_);
}
