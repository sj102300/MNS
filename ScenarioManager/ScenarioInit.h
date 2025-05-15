#pragma once

#include "ScenarioManager.h"
#include "ScenarioInfoPrinter.h"

class ScenarioInit {
public:
    ScenarioInit(const std::string& listen_address,
        const std::string& server_url,
        const std::string& client_id);

    void run();

    // 시나리오 정보 접근용 getter
    ScenarioInfo getScenarioInfo() const;
    Coordinate getBatteryLocation() const;
    std::vector<AircraftInfo> getAircraftList() const;

private:
    std::string listen_address_;
    std::string client_id_;
    std::shared_ptr<ScenarioManager> scenario_manager_;
    ScenarioInfoPrinter printer_;

    void handleStartSignal(const std::string& scenario_id);
};
