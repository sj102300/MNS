#pragma once

#include "ShootDownThread.h"
#include "AircraftManager.h"
#include <vector>
#include <memory>
#include <mutex>
#include <thread>

class ATS {
public:
    void setAircraftList(const std::vector<ats::AircraftInfo>& list);
    void launchAll();
    void terminateAll();

private:
    std::vector<ats::AircraftInfo> aircrafts_;
    ShootDownThread shootDownThread_;
    std::mutex mtx_;
};
