
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

void AircraftSender::sendAircraftMessage() {

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