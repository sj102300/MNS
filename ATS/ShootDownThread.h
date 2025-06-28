#pragma once

#include "Missile.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <utility>
#include <queue>
#include <mutex>

#pragma pack(push, 1)
struct InterceptResultPacket {
    unsigned int EventCode;      // 예: 2003
    unsigned int BodyLength;     // payload 길이
    char EventId[20];            // 예: "SUCCESS"
    char AirCraftId[8];          // 항공기 ID
    char MissileId[8];           // 미사일 ID
};
#pragma pack(pop)

// 외부 사용 가능한 격추 판단용 데이터 구조
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

// 메인 격추 판단 스레드
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

// 외부 함수 선언
bool ShootDowns(std::pair<double, double> currentPoint, std::pair<double, double> missilePoint);
void sendSuccessInfo(std::string aircraftId, std::string missileId);
void initializeMultiSenderSocket();
