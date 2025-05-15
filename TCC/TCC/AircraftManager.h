#pragma once

#include "AircraftReceiver.h"
#include "AircraftSender.h"
#include "EngagementManager.h"
#include "Aircraft.h"
#include <unordered_map>
#include <queue>

class AircraftManager {
public:
    typedef struct _new_aircraft_data {
        std::string aircraftId_;
        TCC::Position location_;
        bool isEnemy_;
    } NewAircraft;

    AircraftManager(IAircraftReceiver* recv, IAircraftSender* sender);
    void updateAircraftPosition();
    void updateAircraftStatus();
    void pushNewAircraftQueue(NewAircraft& newAircraft);

private:
    bool popNewAircraftQueue(NewAircraft& newAircraft);
    void judgeEngagable();
    bool isExistAircraft(std::string& aircraftId);
    void addAircraft(NewAircraft &msg);

    IAircraftReceiver* receiver_;
    IAircraftSender* sender_;
    EngagementManager* engagementManager_;
    std::unordered_map<std::string, Aircraft*> aircrafts_;
    std::thread workThread_;
    std::mutex mtx_;
    std::queue<NewAircraft> newAircraftQueue_;
};