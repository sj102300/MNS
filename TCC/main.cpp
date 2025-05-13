#include <iostream>

#include "AircraftManager.h"

int main() {

    IAircraftReceiver* aircraftReceiver = new AircraftReceiver(std::string("239.0.0.2"), 9999);
    IAircraftSender* aircraftSender = new AircraftSender(std::string("127.0.0.1"), 8081);
    AircraftManager aircraftmanager(aircraftReceiver, aircraftSender);

    aircraftmanager.updateAircraftPosition();
    
    return 0;
}
