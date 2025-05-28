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
#include "UdpMulticastSender.h"

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
	TCC::UdpMulticastSender* multiSender_;
	TCC::UdpSender* udpSender_;
	TCC::UdpReceiver* udpReceiver_;
	MissileManager* missileManager_;
	std::condition_variable cv_;
};