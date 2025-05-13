
#include "AircraftSender.h"

AircraftSender::AircraftSender(const std::string& ip_address, int port) : TcpSender(ip_address, port), len(64) {
	memcpy(buffer, 0, sizeof(buffer));
}

void AircraftSender::sendAircraftData() {
	if (!init()) {
		std::cout << "AircraftSender init() Failed\n";
		return;
	}

	if (!connectToServer()) {
		std::cout << "AircraftSender connectToServer() Failed\n";
		return;
	}

	IAircraftSender::AircraftMSG msg;

	while (true) {
		if (popSendQueue(msg)) {
			writeToBuffer(msg);
			sendByteData(buffer, len);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

void AircraftSender::writeToBuffer(IAircraftSender::AircraftMSG& msg) {
	memcpy(buffer, reinterpret_cast<const void*>(&msg), len);
}

void AircraftSender::pushSendQueue(IAircraftSender::AircraftMSG & msg) {
	std::lock_guard<std::mutex> lock(mtx_);
	sendQueue.push(msg);
}

bool AircraftSender::popSendQueue(IAircraftSender::AircraftMSG& msg) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (sendQueue.empty()) {
		return false;
	}
	msg = sendQueue.front();
	sendQueue.pop();
	return true;
}