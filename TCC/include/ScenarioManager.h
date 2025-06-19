#pragma once

#include "HttpClient.h"
#include "HttpServer.h"
#include "ScenarioInfoPrinter.h"
#include <functional>
#include <mutex>

namespace sm {
    class ScenarioManager {
    public:
        ScenarioManager(const std::string& listen_address,
            const std::string& server_url,
            const std::string& client_id);

        void run();
        void handleStartSignal(const std::string& scenario_id);
        void handleQuitSignal();

        void setOnReadyCallback(std::function<void()> cb);
        void setOnQuitCallback(std::function<void()> cb);

        ScenarioInfo getScenarioInfo() const;
        Coordinate getBatteryLocation() const;
        std::vector<AircraftInfo> getAircraftList() const;

    private:
        HttpServer http_server_;
        HttpClient http_client_;
        ScenarioInfoPrinter printer_;

        std::function<void()> on_start_cb_;
        std::function<void()> on_quit_cb_;
        bool is_running_ = false;
        std::mutex mutex_;
        std::string client_id_;
    };
}