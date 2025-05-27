
#include "ScenarioManager.h"

#define RELEASE 0
#if RELEASE
#define AIRCRAFT_SENDER_IP "192.168.2.7"
#else 
#define AIRCRAFT_SENDER_IP "127.0.0.1"
#endif

ScenarioManager::ScenarioManager() {}

bool ScenarioManager::startScenario() {

    if (!createObjects()) {
        std::cout << "startScenario() Failed\n";
        return false;
    }

    multiReceiver_->start();
    aircraftManager_->start();

    return true;
}

bool ScenarioManager::createObjects() {

    multiReceiver_ = new TCC::UdpMulticastReceiver("239.0.0.1", 9000);
    udpSender_ = new TCC::UdpSender("192.168.2.200", 9000); //OCC �ּ�
    aircraftManager_ = new AircraftManager();
    missileManager_ = new MissileManager(udpSender_);
    //engagementManager_ = new EngagementManager();
    //

    if (!aircraftManager_->init(udpSender_, engagementManager_)) {
        std::cout << "aircraftManager init() Failed\n";
        return false;
    }
    if (!multiReceiver_->init(aircraftManager_, missileManager_)) {
        std::cout << "multiReceiver init() Failed\n";
        return false;
    }
    if (!udpSender_->init()) {
        std::cout << "udpSender init() Failed\n";
        return false;
    }

    return true;
}

void ScenarioManager::quitScenario() {
    delete aircraftManager_;
    delete engagementManager_;
    delete udpSender_;
    delete multiReceiver_;
}