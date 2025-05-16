#pragma once

#include "ScenarioManager.h"
#include "ScenarioInfoPrinter.h"
#include <functional>

namespace sm {
    class ScenarioInit {
    public:
        ScenarioInit(const std::string& listen_address,
            const std::string& server_url,
            const std::string& client_id);

        void run();

        void setOnReadyCallback(std::function<void()> cb);  // 콜백 등록 함수
        void setOnQuitCallback(std::function<void()> cb);  // 종료 콜백 등록
        void handleQuitSignal();

        ScenarioInfo getScenarioInfo() const;
        Coordinate getBatteryLocation() const;
        std::vector<AircraftInfo> getAircraftList() const;

    private:
        std::string listen_address_;
        std::string client_id_;
        std::shared_ptr<ScenarioManager> scenario_manager_;
        ScenarioInfoPrinter printer_;

        std::function<void()> on_start_cb_;  // 시작 콜백 저장
        std::function<void()> on_quit_cb_;  // 종료 콜백 저장

        void handleStartSignal(const std::string& scenario_id);
        
    };
}