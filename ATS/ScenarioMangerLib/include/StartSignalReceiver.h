#pragma once

#include <string>
#include <functional>

void setup_start_signal_listener(
    const std::string& address,
    const std::string& client_id,
    std::function<void(const std::string& scenario_id)> on_start_callback
);
