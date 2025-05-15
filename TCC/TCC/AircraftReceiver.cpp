
#include "AircraftReceiver.h"

AircraftReceiver::AircraftReceiver(const std::string& multicastIp, int port) : UdpMulticastReceiver(multicastIp, port) {
	std::cout << "AircraftReceiver created\n";
}

void AircraftReceiver::getAircraftData() {
	if (!init()) {
		std::cout << "AircraftReceiver Init() Failed\n";
		return;
	}

	recvThread_ = std::thread(&AircraftReceiver::receive, this);

	return;
}

void AircraftReceiver::receive() {
	char buffer[33];
	sockaddr_in senderAddr;
	int addrLen = sizeof(senderAddr);

	AircraftMSG msg;

	while (true) {
		int bytesReceived = recvfrom(serverSocket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr, &addrLen);

		if (bytesReceived < 0) {
			std::cerr << "recvfrom Failed. Error: " << WSAGetLastError() << "\n";
			break;
		}

		if (parseMsg(msg, buffer, sizeof(buffer))) {
			std::cout << "AircraftReceiver received bad packet\n";
			continue;
		}
		pushRecvQueue(msg);
	}
}

bool AircraftReceiver::parseMsg(AircraftMSG & msg, const char* buffer, const int length) {
	memcpy(msg.aircraftId, buffer, 8);
	if (!TCC::isValidAircraftId(msg.aircraftId)) {
		return false;
	}
	
	msg.location.latitude_ = *(double*)(buffer + 8);
	msg.location.longitude_ = *(double*)(buffer + 16);
	msg.location.altitude_ = *(double*)(buffer + 24);
	if (msg.location.isValidPosition()) {
		return false;
	}

	char friendOrEnemy = *(char*)(buffer + 32);
	if (friendOrEnemy == 'E') {
		msg.isEnemy = true;
	}
	else if (friendOrEnemy == 'O') {
		msg.isEnemy = false;
	}
	else {
		return false;
	}

	return true;
}

void AircraftReceiver::pushRecvQueue(AircraftMSG &msg) {
	std::lock_guard<std::mutex> lock(mtx_);
	recvQueue_.push(msg);
}

bool AircraftReceiver::popRecvQueue(AircraftMSG& msg) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (recvQueue_.empty()) {
		return false;
	}
	msg = recvQueue_.front();
	recvQueue_.pop();
	return true;
}

AircraftReceiver::~AircraftReceiver() {
	if (recvThread_.joinable()) {
		recvThread_.join(); // 안전하게 종료 대기
	}
	closesocket(serverSocket_);
}