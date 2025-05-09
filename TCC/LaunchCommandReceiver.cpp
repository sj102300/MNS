
#include "LaunchCommandReceiver.h"

namespace LaunchCommand {

	void LaunchCommandReceiver::parseMsg(LaunchCommandMSG &msg, const char* buffer, int length) {
	
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
		connectClient();

		std::thread recvThread([this]() {
			this->receive();
			});

		recvThread.join();
	}

	void LaunchCommandReceiver::response() {

	}
}
