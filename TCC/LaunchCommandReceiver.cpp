
#include "LaunchCommandReceiver.h"

LaunchCommandReceiver::LaunchCommandReceiver(std::string ip_address, int port) : TCC::TcpReceiver(ip_address, port) {

}

void LaunchCommandReceiver::start() {
	if (!init()) {
		std::cout << "LaunchCommandReceiver Init() Failed\n";
		return;
	}

	if (!connectClient()) {
		std::cout << "LaunchCommandReceiver connectClient() Failed\n";
		return;
	}

	std::thread recvThread([this]() {
		this->receive();
		});

	recvThread.join();
}

void LaunchCommandReceiver::receive() {

	char buffer[28] = { 0 };
	LaunchCommandMSG msg;

	while (true) {
		int bytesReceived = recv(clientSocket_, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			if (!parseMsg(msg, buffer, bytesReceived)) {
				std::cout << "LaunchCommandReceiver received bad packet\n";
				continue;
			}

			//work();

			response();
		}
		else if (bytesReceived == 0) {
			std::cout << "Client Disconnected\n";
			break;
		}
		else {
			std::cerr << "recv() Failed\n";
			break;
		}
	}
	closesocket(clientSocket_);
}

bool LaunchCommandReceiver::parseMsg(LaunchCommandMSG& msg, const char* buffer, const int length) {
	memcpy(msg.aircraftId, buffer, 8);
	if (!TCC::isValidAircraftId(msg.aircraftId)) {
		return false;
	}
	msg.launchCommandId = std::string(buffer + 8, 20);
	if (!TCC::isValidCommand(msg.launchCommandId)) {
		return false;
	}
	return true;
}

void LaunchCommandReceiver::work(LaunchCommandMSG& msg) {

}

void LaunchCommandReceiver::response() {

}
