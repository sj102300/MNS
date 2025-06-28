#pragma once

#include "ShootDownThread.h"
#include "AircraftWorker.h"
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
    std::vector<std::shared_ptr<ats::AircraftWorker>> workers_;
    std::vector<std::thread> workerThreads_;
    std::mutex mtx_;

    ShootDownThread shootDownThread_;  // ���� �Ǵ� ������ (�� ���� ������)
};
