#include "ATS.h"
#include <iostream>

void ATS::setAircraftList(const std::vector<ats::AircraftInfo>& list) {
    aircrafts_ = list;
}

void ATS::launchAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    for (const auto& info : aircrafts_) {
        auto worker = std::make_shared<ats::AircraftWorker>(info);
        std::thread t(std::ref(*worker));

        workers_.push_back(worker);
        workerThreads_.emplace_back(std::move(t));

        //std::cout << "[ATS] 스레드 시작 → ID: " << info.id << "\n";
    }
}

void ATS::terminateAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    for (auto& worker : workers_) {
        worker->stop();
    }

    for (auto& t : workerThreads_) {
        if (t.joinable()) t.join();
    }

    workers_.clear();
    workerThreads_.clear();

    //std::cout << "[ATS] 모든 스레드 종료 완료\n";
}
