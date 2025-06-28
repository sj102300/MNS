#pragma once

#include "AircraftWorker.h"
#include <vector>
#include <string>
#include <thread>
#include <memory>
#include <mutex>

class ATS {
public:
    void setAircraftList(const std::vector<ats::AircraftInfo>& list);
    void launchAll();
    void terminateAll();

private:
    std::vector<ats::AircraftInfo> aircrafts_;
    std::vector<std::thread> workerThreads_;
    std::vector<std::shared_ptr<ats::AircraftWorker>> workers_;
    std::mutex mtx_;
};
