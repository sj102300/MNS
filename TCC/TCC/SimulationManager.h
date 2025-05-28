#pragma once

#include <string>
#include <atomic>
#include <condition_variable>

#include "share.h"
#include "AircraftManager.h"
#include "EngagementManager.h"
#include "UdpMulticastReceiver.h"
#include "UdpSender.h"
#include "MissileManager.h"
#include "ScenarioManager.h"

class SimulationManager{
public:
	SimulationManager();
	void startSimulation();
	void quitSimulation();

private:
	bool startScenario();
	bool quitScenario();
	bool createObjects();

	std::string scenarioId_;
	TCC::Position batterLoc_;
	std::atomic<bool> isRunning_;
	std::atomic<bool> isChanged_;
	std::mutex mtx_;

	sm::ScenarioManager* scenarioManager_;
	AircraftManager* aircraftManager_;
	EngagementManager* engagementManager_;
	TCC::UdpMulticastReceiver* multiReceiver_;
	TCC::UdpSender* udpSender_;
	MissileManager* missileManager_;
	std::condition_variable cv_;
};

//const std::string SUBSYSTEM_ID = "MFR";
//const std::string SCN_LOCAL_IP = "http://192.168.2.31:8080";  // 荐脚 林家
//const std::string SCN_SERVER_IP = "http://192.168.2.30:8080";  // SCN 辑滚 林家