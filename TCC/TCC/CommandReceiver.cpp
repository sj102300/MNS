
#include "CommandReceiver.h"

CommandReceiver::CommandReceiver(std::string ip_address, int port) : TcpReceiver(ip_address, port) {

}

void CommandReceiver::receiveMessage() {
	if (!init()) {

	}
}

void CommandReceiver::recvModeChangeCommand() {
	//EngagementManager::changeMode() 실행
}

void CommandReceiver::recvManualFireCommand() {
	//EngagementManager::launchMissile() 실행
}

void CommandReceiver::recvEmergencyDestroyCommand() {
	//EngagementManager::emergencyDestroy() 실행
}