#pragma once

#include "AircraftReceiver.h"
#include "AircraftSender.h"
#include "Aircraft.h"
#include <unordered_map>

class AircraftManager {
public:
    AircraftManager(IAircraftReceiver* recv, IAircraftSender* sender);
     
    void updateAircraftPosition();
    void updateAircraftStatus();

private:
    void judgeEngagable();
    bool isExistAircraft(std::string& aircraftId);
    void addAircraft(std::string& aircraftId, IAircraftReceiver::AircraftMSG &msg);

    IAircraftReceiver* receiver_;
    IAircraftSender* sender_;
    std::thread workThread_;
    std::unordered_map<std::string, Aircraft*> aircrafts_;
};