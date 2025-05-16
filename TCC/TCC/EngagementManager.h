#pragma once

//#include "CommandReceiver.h"
#include <unordered_map>
#include <string>
#include <queue>

class EngagementManager {
public:
	void startEngagementSimulation();
	void mappingMissileToAircraft(std::string& aircraftId);
	bool isHitTarget();
	bool changeMode();
	void launchMissile(std::string& missileId);
	void emergencyDestroy(std::string& missileId);
	void addEngagableAircraft(std::string& aircraftId);

private:
	//ICommandReceiver* commandReceiver_;
	//ISuccessReceiver* successReceiver_;
	//ICommandSender* commandSender_;
	std::unordered_map<std::string, std::string> missileToAircraft_;
	unsigned int mode_;
	std::queue<std::string> engagableAircrafts_;
};