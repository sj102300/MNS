#pragma once

#include "AircraftReceiver.h"
#include "AircraftSender.h"
#include "EngagementManager.h"
#include "Aircraft.h"
#include <unordered_map>
#include <queue>

class AircraftManager {
public:
    AircraftManager(IAircraftReceiver* recv, IAircraftSender* sender);
    void startAircraftSimulation();

private:
    void judgeEngagable();
    bool isExistAircraft(std::string& aircraftId);
    void addAircraft(IAircraftReceiver::NewAircraft &msg);

    IAircraftReceiver* receiver_;
    IAircraftSender* sender_;
    EngagementManager* engagementManager_;
    std::unordered_map<std::string, Aircraft*> aircrafts_;
    std::thread workThread_;
    std::mutex mtx_;
};