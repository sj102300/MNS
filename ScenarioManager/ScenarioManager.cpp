#include "ScenarioManager.h"
#include <windows.h>
#include <iostream>

namespace sm {
    ScenarioManager::ScenarioManager(const std::string& listen_address,
        const std::string& server_url,
        const std::string& client_id)
        : http_server_(listen_address, client_id),
        http_client_(server_url),
        client_id_(client_id) {
    }

    void ScenarioManager::run() {
        SetConsoleOutputCP(CP_UTF8);

        http_server_.setOnStartCallback([this](const std::string& scenario_id) {
            handleStartSignal(scenario_id);
            });

        http_server_.setOnQuitCallback([this]() {
            handleQuitSignal();
            });

        http_server_.start();

        std::cout << u8"[" << client_id_ << u8"] OCC ���۽�ȣ�� ��ٸ��ϴ�.\n";
    }

    void ScenarioManager::setOnReadyCallback(std::function<void()> cb) {
        on_start_cb_ = std::move(cb);
    }

    void ScenarioManager::setOnQuitCallback(std::function<void()> cb) {
        on_quit_cb_ = std::move(cb);
    }

    void ScenarioManager::handleStartSignal(const std::string& scenario_id) {
        bool loaded = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            std::cout << u8"[" << client_id_ << u8"] �ó����� ��û ����\n";
            if (is_running_) {
                std::cout << u8"[" << client_id_ << u8"] �̹� ���� �� - ���� ��ȣ ����\n";
                return;
            }

            if (!http_client_.requestScenario(scenario_id)) {
                std::cerr << u8"[" << client_id_ << u8"] �ó����� ��û ����\n";
                return;
            }

            is_running_ = true;
            loaded = true;
            std::cout << u8"[" << client_id_ << u8"] �ó����� ����\n";
        }

        if (loaded && on_start_cb_) {
            on_start_cb_();
        }

        printer_.printInfo(http_client_);
        printer_.printBattery(http_client_);
        printer_.printAircraftList(http_client_);
    }

    void ScenarioManager::handleQuitSignal() {
        bool was_running = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << u8"[" << client_id_ << u8"] OCC ���� ��ȣ ���� �� �ó����� ���� ó��\n";
            std::cout << u8"[DEBUG] ���� ���� is_running_: " << (is_running_ ? u8"true" : u8"false") << "\n";

            if (!is_running_) {
                std::cout << u8"[" << client_id_ << u8"] ���� ���� �ƴ� - ���� ��ȣ ����\n";
                return;
            }

            http_client_.clearState();
            is_running_ = false;
            was_running = true;

            std::cout << u8"[" << client_id_ << u8"] �ó����� ���� �� ���� �ʱ�ȭ �Ϸ�\n";
        }

        if (was_running && on_quit_cb_) {
            on_quit_cb_();
        }
    }

    ScenarioInfo ScenarioManager::getScenarioInfo() const {
        return http_client_.getScenarioInfo();
    }

    Coordinate ScenarioManager::getBatteryLocation() const {
        return http_client_.getBatteryLocation();
    }

    std::vector<AircraftInfo> ScenarioManager::getAircraftList() const {
        return http_client_.getAircraftList();
    }
}
