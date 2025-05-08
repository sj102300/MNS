#pragma once

#include "share.h"
#include <string>

//싱글톤으로 구현? static 으로 구현?
class Scenario {
public:
	TCC::Position getBatteryLocation();
	void getInstance();

private:
	TCC::Position BatteryLocation;
	std::string scenarioId;
};