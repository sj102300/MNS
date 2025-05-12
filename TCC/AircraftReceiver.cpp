
#include "AircraftReceiver.h"

AircraftReceiver::AircraftReceiver(std::string multicastIp, int port) : UdpMulticastReceiver(multicastIp, port) {

}

void AircraftReceiver::receiveLoop() {
	char buffer[33];
	sockaddr_in senderAddr;
	int addrLen = sizeof(senderAddr);

	AircraftMSG msg;

	while (true) {
		int bytesReceived = recvfrom(serverSocket_, buffer, sizeof(buffer), 0,
			(struct sockaddr*)&senderAddr, &addrLen);

		if (bytesReceived < 0) {
			std::cerr << "recvfrom Failed\n";
			break;
		}

		parseMsg(msg, buffer, sizeof(buffer));

		//response();
	}
}

void AircraftReceiver::parseMsg(AircraftMSG & msg, const char* buffer, int length) {
	//msg.aircraftId = buffer
}

void AircraftReceiver::start() {

	if (!init()) {
		std::cout << "AircraftReceiver Init() Failed\n";
		return;
	}

	std::thread recvThread([this]() {
		this->receiveLoop();
		});

	recvThread.join();

}