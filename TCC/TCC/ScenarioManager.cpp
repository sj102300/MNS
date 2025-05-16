
#include "ScenarioManager.h"

#define RELEASE 0
#if RELEASE
#define AIRCRAFT_SENDER_IP "192.168.2.7"
#else 
#define AIRCRAFT_SENDER_IP "127.0.0.1"
#endif

ScenarioManager::ScenarioManager() {}

void ScenarioManager::startScenario() {
    createAircraftManager();
    createEngagementManager();
}

void ScenarioManager::createAircraftManager() {
    IAircraftReceiver* aircraftReceiver = new AircraftReceiver(std::string("239.0.0.2"), 9999);
    IAircraftSender* aircraftSender = new AircraftSender(std::string(AIRCRAFT_SENDER_IP), 9000);
    aircraftManager_ = new AircraftManager(aircraftReceiver, aircraftSender);
    aircraftManager_->startAircraftSimulation();
    return;
}

void ScenarioManager::createEngagementManager() {

    //ICommandReceiver* commandReceiver = new CommandReceiver(std::string("192.168.2.7"), 9000);
    //ISuccessReceiver* successReceiver_;
    //ICommandSender* commandSender_;
    //engagementManager_->startEngagementSimulation();
}

void ScenarioManager::quitScenario() {

}