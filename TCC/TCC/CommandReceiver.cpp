
#include "CommandReceiver.h"

void CommandReceiver::receiveMessage() {
	if (!init()) {

	}
}

void CommandReceiver::recvModeChangeCommand() {
	//EngagementManager::changeMode() ����
}

void CommandReceiver::recvManualFireCommand() {
	//EngagementManager::launchMissile() ����
}

void CommandReceiver::recvEmergencyDestroyCommand() {
	//EngagementManager::emergencyDestroy() ����
}