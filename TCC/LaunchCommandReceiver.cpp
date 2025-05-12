
#include "LaunchCommandReceiver.h"

LaunchCommandReceiver::LaunchCommandReceiver(std::string ip_address, int port) : TCC::TcpReceiver(ip_address, port) {

}

void LaunchCommandReceiver::parseMsg(LaunchCommandMSG& msg, const char* buffer, int length) {

}

void LaunchCommandReceiver::receive() {

	char buffer[28] = { 0 };
	LaunchCommandMSG msg;

	while (true) {
		int bytesReceived = recv(clientSocket_, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			parseMsg(msg, buffer, bytesReceived);

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

void LaunchCommandReceiver::response() {

}
