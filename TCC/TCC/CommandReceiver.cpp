
#include "CommandReceiver.h"

CommandReceiver::CommandReceiver(std::string ip_address, int port) : TcpReceiver(ip_address, port) {

}

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