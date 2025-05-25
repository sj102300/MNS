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

        std::cout << u8"[" << client_id_ << u8"] OCC ���۽�ȣ�� ��ٸ��ϴ�.\n";
    }

    void ScenarioInit::setOnReadyCallback(std::function<void()> cb) {
        on_start_cb_ = std::move(cb);
    }

    void ScenarioInit::handleStartSignal(const std::string& scenario_id) {
        bool loaded = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            std::cout << u8"[" << client_id_ << u8"] �ó����� ��û ����\n";
            if (is_running_) {
                std::cout << u8"[" << client_id_ << u8"] �̹� ���� �� - ���� ��ȣ ����\n";
                return;
            }

            if (!scenario_manager_->requestScenario(scenario_id)) {
                std::cerr << u8"[" << client_id_ << u8"] �ó����� ��û ����\n";
                return;
            }

            is_running_ = true;
            loaded = true;
            std::cout << u8"[" << client_id_ << u8"] �ó����� ����\n";
        }

        if (loaded && on_start_cb_) {
            on_start_cb_();  // �ݹ� �ٱ����� ȣ��
        }

        printer_.printInfo(*scenario_manager_);
        printer_.printBattery(*scenario_manager_);
        printer_.printAircraftList(*scenario_manager_);
    }

    void ScenarioInit::setOnQuitCallback(std::function<void()> cb) {
        on_quit_cb_ = std::move(cb);
    }

    void ScenarioInit::handleQuitSignal() {
        bool was_running = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << u8"[" << client_id_ << u8"] OCC ���� ��ȣ ���� �� �ó����� ���� ó��\n";
            std::cout << u8"[DEBUG] ���� ���� is_running_: " << (is_running_ ? u8"true" : u8"false") << "\n";

            if (!scenario_manager_) {
                std::cerr << u8"[ERROR] ScenarioManager ��ü�� nullptr�Դϴ�. �ʱ�ȭ�� �ʿ��մϴ�.\n";
                return;
            }

            if (!is_running_) {
                std::cout << u8"[" << client_id_ << u8"] ���� ���� �ƴ� - ���� ��ȣ ����\n";
                return;
            }

            if (scenario_manager_) {
                scenario_manager_->clearState();  // ��ü�� �״�� �ΰ�, ���� ���¸� �ʱ�ȭ
            }
            is_running_ = false;
            was_running = true;

            std::cout << u8"[" << client_id_ << u8"] �ó����� ���� �� ���� �ʱ�ȭ �Ϸ�\n";
        }

        if (was_running && on_quit_cb_) {
            on_quit_cb_();  // �� �ٱ����� �����ϰ� �ݹ� ȣ��
        }
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
