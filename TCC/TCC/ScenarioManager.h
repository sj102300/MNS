#pragma once

#include <string>
#include <atomic>
#include "share.h"
#include "AircraftManager.h"
#include "EngagementManager.h"
#include "MissileManager.h"

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
	void startScenario();
	void quitScenario();

private:

	void createAircraftManager();
	void createEngagementManager();
	void createMissileManager();

	std::string scenarioId_;
	TCC::Position batterLoc_;
	std::atomic<bool> isRunning_;

	AircraftManager* aircraftManager_;
	EngagementManager* engagementManager_;
	MissileManager* missileManager_;

	//MissileManager* missileManager_;
};