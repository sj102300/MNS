#pragma once

#include <string>
#include <atomic>
#include "share.h"
#include "AircraftManager.h"
#include "EngagementManager.h"
#include "UdpMulticastReceiver.h"
#include "UdpSender.h"

//class IScenarioReceiver {
//public:
//	virtual void recvScenarioStartSignal() = 0;
//	virtual void recvScenarioQuitSignal() = 0;
//};

//class IScenarioSender {
//public:
//	virtual void getScenarioInfo() = 0;
//};

class ScenarioManager{
public:
	ScenarioManager();
	bool startScenario();
	void quitScenario();

private:
	bool createObjects();

	std::string scenarioId_;
	TCC::Position batterLoc_;
	std::atomic<bool> isRunning_;

	AircraftManager* aircraftManager_;
	EngagementManager* engagementManager_;
	TCC::UdpMulticastReceiver* multiReceiver_;
	TCC::UdpSender* udpSender_;

	//MissileManager* missileManager_;
};