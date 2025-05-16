#include "ScenarioInit.h"
#include "StartSignalReceiver.h"
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

        setup_start_signal_listener(
            listen_address_,
            client_id_,
            [this](const std::string& scenario_id) {
                handleStartSignal(scenario_id);
            }
        );

        std::cout << u8"[" << client_id_ << u8"] OCC에서 시작신호를 기다립니다.\n";
    }

    void ScenarioInit::setOnReadyCallback(std::function<void()> cb) {
        on_ready_cb_ = std::move(cb);
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

        if (on_ready_cb_) {
            on_ready_cb_();  // 외부에서 등록한 콜백 호출
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
