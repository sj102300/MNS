#include <iostream>

#include "AircraftManager.h"

#define DEVELOP 1

#if DEVELOP
#define AIRCRAFT_SENDER_IP "192.168.2.7"

#else 
#define AIRCRAFT_SENDER_IP "127.0.0.1"

#endif

int main() {

    IAircraftReceiver* aircraftReceiver = new AircraftReceiver(std::string("239.0.0.2"), 9999);
    IAircraftSender* aircraftSender = new AircraftSender(std::string(AIRCRAFT_SENDER_IP), 9000);
    AircraftManager aircraftmanager(aircraftReceiver, aircraftSender);

    aircraftmanager.updateAircraftPosition();
    
    return 0;
}
