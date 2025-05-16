#pragma once

#include <string>
#include <functional>

namespace sm {
    void setup_scenario_command_listener(
        const std::string& address,
        const std::string& client_id,
        std::function<void(const std::string& scenario_id)> on_start_callback,
        std::function<void()> on_quit_callback
    );
}
