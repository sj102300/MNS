#include "ScenarioInit.h"
#include "ScenarioCommandReceiver.h"
#include <windows.h>
#include <iostream>

namespace sm {
    ScenarioInit::ScenarioInit(const std::string& listen_address,
        const std::string& server_url,
        const std::string& client_id)
        : listen_address_(listen_address), client_id_(client_id),
        scenario_manager_(std::make_shared<ScenarioManager>(server_url)) {
    }

    void ScenarioInit::run() {
        SetConsoleOutputCP(CP_UTF8);

        setup_scenario_command_listener(
            listen_address_,
            client_id_,
            [this](const std::string& scenario_id) {
                handleStartSignal(scenario_id);
            },
            [this]() {
                handleQuitSignal();  // ���� �ݹ� ���
            }
        );

        std::cout << u8"[" << client_id_ << u8"] OCC���� ���۽�ȣ�� ��ٸ��ϴ�.\n";
    }

    void ScenarioInit::setOnReadyCallback(std::function<void()> cb) {
        on_start_cb_ = std::move(cb);
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

        if (on_start_cb_) {
            on_start_cb_();  // �ܺο��� ����� �ݹ� ȣ��
        }
    }

    void ScenarioInit::setOnQuitCallback(std::function<void()> cb) {
        on_quit_cb_ = std::move(cb);
    }

    void ScenarioInit::handleQuitSignal() {
        std::cout << u8"[" << client_id_ << u8"] OCC ���� ��ȣ ���� �� �ó����� ���� ó��\n";

        if (on_quit_cb_) {
            on_quit_cb_();  // �ݹ� ����
        }


        // TODO: �ó����� ���� ����, ������ ���� ��
    }

    ScenarioInfo ScenarioInit::getScenarioInfo() const {
        return scenario_manager_->getScenarioInfo();
    }

    Coordinate ScenarioInit::getBatteryLocation() const {
        return scenario_manager_->getBatteryLocation();
    }

    std::vector<AircraftInfo> ScenarioInit::getAircraftList() const {
        return scenario_manager_->getAircraftList();
    }
}
