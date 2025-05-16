
#include "AircraftReceiver.h"

AircraftReceiver::AircraftReceiver(const std::string& multicastIp, int port) : UdpMulticastReceiver(multicastIp, port) {
	std::cout << "AircraftReceiver created\n";
}

void AircraftReceiver::recvAircraftData() {
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
	NewAircraft newAircraft;
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

		newAircraft.aircraftId_ = std::string(msg.aircraftId, 8);
		newAircraft.location_ = msg.location;
		newAircraft.isEnemy_ = msg.friend_or_our == 'E';

		pushRecvQueue(newAircraft);
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
	if (!msg.location.isValidPosition()) {
		return false;
	}

	char friendOrEnemy = *(char*)(buffer + 32);
	if (friendOrEnemy != 'E' || friendOrEnemy != 'O')
		return false;

	return true;
}


void AircraftReceiver::pushRecvQueue(NewAircraft& newAircraft) {
	std::lock_guard<std::mutex> lock(mtx_);
	recvQueue_.push(newAircraft);
}

bool AircraftReceiver::popRecvQueue(NewAircraft& newAircraft) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (recvQueue_.empty()) {
		return false;
	}
	newAircraft = recvQueue_.front();
	recvQueue_.pop();
	return true;
}


AircraftReceiver::~AircraftReceiver() {
	if (recvThread_.joinable()) {
		recvThread_.join(); // �����ϰ� ���� ���
	}
	closesocket(serverSocket_);
}