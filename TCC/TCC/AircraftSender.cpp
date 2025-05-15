
#include "AircraftSender.h"

AircraftSender::AircraftSender(const std::string& ip_address, int port) : TcpSender(ip_address, port), len(64) {
	memset(buffer, 0, sizeof(buffer));
	std::cout << "AircraftSender created\n";
}

void AircraftSender::sendAircraftData() {
	if (!init()) {
		std::cout << "AircraftSender init() Failed\n";
		return;
	}

	bool connected = false;
	for (int _ = 0; _ < 5; ++_) {
		if (connectToServer()) {
			std::cout << "AircraftSender connectToServer() success\n";
			connected = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1)); // 1ÃÊ ´ë±â
	}
	if (!connected) {
		std::cerr << "AircraftSender connectToServer() failed after 5 attempts\n";
		return;
	}

	sendThread_ = std::thread(&AircraftSender::sendAircraftMessage, this);
}


void AircraftSender::pushSendQueue(NewAircraftWithIP& newAircraftWithIp) {
	std::lock_guard<std::mutex> lock(mtx_);
	sendQueue.push(newAircraftWithIp);
}
bool AircraftSender::popSendQueue(NewAircraftWithIP& newAircraftWithIp) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (sendQueue.empty())
		return false;
	newAircraftWithIp = sendQueue.front();
	sendQueue.pop();
	return true;
}

void AircraftSender::sendAircraftMessage() {

	NewAircraftWithIP newAircraftWithIp;
	while (true) {
		if (popSendQueue(newAircraftWithIp)) {
			writeToBuffer(newAircraftWithIp);
			sendByteData(buffer, len);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

void AircraftSender::writeToBuffer(NewAircraftWithIP& msg) {
	memcpy(buffer, msg.aircraftId_.c_str(), 8);
	memcpy(buffer+8, static_cast<void*>(&(msg.location_)), sizeof(TCC::Position));
	*(unsigned int*)(buffer + 32) = msg.isEnemy_;
	*(unsigned int*)(buffer + 36) = msg.engagementStatus_;
	memcpy(buffer + 40, static_cast<void*>(&(msg.impactPoint_)), sizeof(TCC::Position));
}