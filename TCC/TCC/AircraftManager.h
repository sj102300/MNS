#pragma once

#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>

#include "EngagementManager.h"
#include "Aircraft.h"

namespace TCC {
    class UdpSender;
}

class AircraftManager {
public:
    typedef struct _new_aircraft {
        std::string aircraftId_;
        TCC::Position location_;
        bool isEnemy_;
    }NewAircraft;

    typedef struct _new_aircraft_data_with_IP_data {
        NewAircraft aircraftData_;
        unsigned int engagementStatus_;
        TCC::Position impactPoint_;
    } NewAircraftWithIP;
    
    AircraftManager();
    bool init(TCC::UdpSender* sender, EngagementManager* engagementManager);
    void start();
    void handleReceivedAircraft(NewAircraft& newAircraft);
    ~AircraftManager();

private:
    void judgeEngagable();
    bool isExistAircraft(std::string& aircraftId);
    void addAircraft(NewAircraft& msg);
    void pushNewAircraftQueue(NewAircraft& newAircraft);
    bool popNewAircraftQueue(NewAircraft& newAircraft);

    TCC::UdpSender* sender_;
    EngagementManager* engagementManager_;
    std::queue<NewAircraft> newAircraftQueue_;
    std::unordered_map<std::string, Aircraft*> aircrafts_;
    std::thread workThread_;
    std::mutex mtx_;
};