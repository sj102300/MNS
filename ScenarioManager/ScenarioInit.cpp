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
                handleQuitSignal();  // 종료 콜백 등록
            }
        );

        std::cout << u8"[" << client_id_ << u8"] OCC 시작신호를 기다립니다.\n";
    }

    void ScenarioInit::setOnReadyCallback(std::function<void()> cb) {
        on_start_cb_ = std::move(cb);
    }

    void ScenarioInit::handleStartSignal(const std::string& scenario_id) {
        bool loaded = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            std::cout << u8"[" << client_id_ << u8"] 시나리오 요청 시작\n";
            if (is_running_) {
                std::cout << u8"[" << client_id_ << u8"] 이미 실행 중 - 시작 신호 무시\n";
                return;
            }

            if (!scenario_manager_->requestScenario(scenario_id)) {
                std::cerr << u8"[" << client_id_ << u8"] 시나리오 요청 실패\n";
                return;
            }

            is_running_ = true;
            loaded = true;
            std::cout << u8"[" << client_id_ << u8"] 시나리오 시작\n";
        }

        if (loaded && on_start_cb_) {
            on_start_cb_();  // 콜백 바깥에서 호출
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
            std::cout << u8"[" << client_id_ << u8"] OCC 종료 신호 수신 → 시나리오 종료 처리\n";
            std::cout << u8"[DEBUG] 현재 상태 is_running_: " << (is_running_ ? u8"true" : u8"false") << "\n";

            if (!scenario_manager_) {
                std::cerr << u8"[ERROR] ScenarioManager 객체가 nullptr입니다. 초기화가 필요합니다.\n";
                return;
            }

            if (!is_running_) {
                std::cout << u8"[" << client_id_ << u8"] 실행 중이 아님 - 종료 신호 무시\n";
                return;
            }

            if (scenario_manager_) {
                scenario_manager_->clearState();  // 객체는 그대로 두고, 내부 상태만 초기화
            }
            is_running_ = false;
            was_running = true;

            std::cout << u8"[" << client_id_ << u8"] 시나리오 종료 및 상태 초기화 완료\n";
        }

        if (was_running && on_quit_cb_) {
            on_quit_cb_();  // 락 바깥에서 안전하게 콜백 호출
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
