#pragma once

#include "Missile.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <utility>
#include <queue>
#include <mutex>

// �ܺ� ��� ������ ���� �Ǵܿ� ������ ����
struct AircraftSnapshot {
    std::string id;
    std::pair<double, double> currentPoint;
    char IFF;
};

// Thread-safe queue for shootdown requests
class ShootDownQueue {
public:
    void push(const AircraftSnapshot& data);
    bool tryPop(AircraftSnapshot& out);

private:
    std::queue<AircraftSnapshot> queue_;
    std::mutex mtx_;
};

// ���� ���� �Ǵ� ������
class ShootDownThread {
public:
    void start();
    void stop();
    void enqueue(const AircraftSnapshot& snapshot);

private:
    void run();

    ShootDownQueue queue_;
    std::thread worker_;
    std::atomic<bool> running_ = false;
};

// �ܺ� �Լ� ����
bool ShootDowns(std::pair<double, double> currentPoint, std::pair<double, double> missilePoint);
void sendSuccessInfo(std::string aircraftId, std::string missileId);
void initializeMultiSenderSocket();
