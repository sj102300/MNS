#include "ATS.h"

void ATS::setAircraftList(const std::vector<ats::AircraftInfo>& list) {
    aircrafts_ = list;
}

void ATS::launchAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    shootDownThread_.start();  // 먼저 격추 판단 스레드 시작

    for (const auto& info : aircrafts_) {
        auto worker = std::make_shared<ats::AircraftWorker>(info, &shootDownThread_);
        std::thread t(std::ref(*worker));
        workers_.push_back(worker);
        workerThreads_.emplace_back(std::move(t));
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

    shootDownThread_.stop();  // 마지막에 격추 판단 스레드 종료
}