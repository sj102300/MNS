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

        std::cout << u8"[" << client_id_ << u8"] OCC에서 시작신호를 기다립니다.\n";
    }

    void ScenarioInit::setOnReadyCallback(std::function<void()> cb) {
        on_start_cb_ = std::move(cb);
    }

    void ScenarioInit::handleStartSignal(const std::string& scenario_id) {
        std::cout << u8"[" << client_id_ << u8"] 시나리오 요청 시작\n";

        if (!scenario_manager_->requestScenario(scenario_id)) {
            std::cerr << u8"[" << client_id_ << u8"] 요청 실패\n";
            return;
        }

        printer_.printInfo(*scenario_manager_);
        printer_.printBattery(*scenario_manager_);
        printer_.printAircraftList(*scenario_manager_);

        if (on_start_cb_) {
            on_start_cb_();  // 외부에서 등록한 콜백 호출
        }
    }

    void ScenarioInit::setOnQuitCallback(std::function<void()> cb) {
        on_quit_cb_ = std::move(cb);
    }

    void ScenarioInit::handleQuitSignal() {
        std::cout << u8"[" << client_id_ << u8"] OCC 종료 신호 수신 → 시나리오 종료 처리\n";

        if (on_quit_cb_) {
            on_quit_cb_();  // 콜백 실행
        }


        // TODO: 시나리오 상태 리셋, 스레드 정리 등
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
