#pragma once

#include "share.h"
#include <string>

//�̱������� ����? static ���� ����?
class Scenario {
public:
	TCC::Position getBatteryLocation();
	void getInstance();

private:
	TCC::Position BatteryLocation;
	std::string scenarioId;
};