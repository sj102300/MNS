
#include "UdpReceiver.h"
#include "share.h"

TCC::UdpReceiver::UdpReceiver(std::string ip, int port) :ip_(ip), port_(port) {

}

TCC::UdpReceiver::~UdpReceiver() {
	WSACleanup();
}

bool TCC::UdpReceiver::init(EngagementManager * engagementManager) {
	
	if (engagementManager == nullptr)
		return false;
	
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed\n";
		return false;
	}

	serverSocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket_ == INVALID_SOCKET) {
		std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
		return false;
	}

	recvAddr_.sin_family = AF_INET;
	recvAddr_.sin_port = htons(9999);
	recvAddr_.sin_addr.s_addr = inet_addr("192.168.2.189");

	if (bind(serverSocket_, (sockaddr*)&recvAddr_, sizeof(recvAddr_)) == SOCKET_ERROR) {
		std::cout << "Bind failed: " << WSAGetLastError() << "\n";
		return false;
	}
	isRunning_ = true;

	std::cout << "Udpreceiver init() success\n";

	return true;
}

void TCC::UdpReceiver::start() {
	isRunning_ = true;
	recvThread_ = std::thread(&TCC::UdpReceiver::receive, this);
}

void TCC::UdpReceiver::stop() {
	isRunning_ = false;
	closesocket(serverSocket_);

	if (recvThread_.joinable()) {
		recvThread_.join();
	}
	return;
}

void TCC::UdpReceiver::receive() {

	Header header;
	ModeChangeMSG modechangemsg;
	ManualFireMSG manualFireMsg;
	EmergencyDestroyMSG emergencyDestroyMsg;
	int addrLen = sizeof(senderAddr_);

	while (isRunning_) {
		int bytesReceived = recvfrom(serverSocket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr_, &addrLen);

		if (bytesReceived < 0) {
			std::cout << "UdpReceiver: recvfrom Failed. Error: " << WSAGetLastError() << "\n";
			break;
		}

		parseHeader(buffer, header);
		//std::cout << header.commandCode_ << std::endl;

		switch (header.commandCode_) {
		case CommandCode::ModeChangeRequest:
			if (!parseModeChangeMSG(buffer + 8, modechangemsg)) 
				break;
			std::cout << "Modechanged: " << modechangemsg.mode_ << "\n";
			//engagementManager_->changeMode(modechangemsg.mode_);
			responseChangeModeAck(engagementManager_->changeMode(modechangemsg.mode_));
			break;

		case CommandCode::ManualFireRequest:
			std::cout << "ManualFireRequest" << std::endl;
			if (!parseManualFireMSG(buffer + 8, manualFireMsg))
				break;
			engagementManager_->manualFire(std::string(manualFireMsg.commandId_, 20), std::string(manualFireMsg.targetAircraftId_, 8));
			break;

		case CommandCode::EmergencyDestroyRequest:
			std::cout << "EmergencyDestroyRequest" << std::endl;
			if (!parseEmergencyDestroyMSG(buffer + 8, emergencyDestroyMsg))
				break;
			//ack 보내기
			std::cout << "EmergencyDestroyMSG: " << emergencyDestroyMsg.commandId_ << "\n";
			engagementManager_->emergencyDestroy(std::string(emergencyDestroyMsg.commandId_, 20), std::string(emergencyDestroyMsg.targetMissileId_, 8));

			break;

		default:
			break;
		}
	}
}

void TCC::UdpReceiver::responseChangeModeAck(unsigned int changedMode) {
	AckHeader header;
	header.commandCode_ = CommandCode::ModeChangeRequest;
	header.bodyLength_ = 4;
	ModeChangeAck body;
	body.mode_ = changedMode;

	char buffer[12];
	//응답 에코하기    
	memcpy(buffer, &header, sizeof(AckHeader));
	memcpy(buffer + 8, &body, sizeof(ModeChangeAck));

	int sent = sendto(serverSocket_, buffer, sizeof(buffer), 0,
		reinterpret_cast<const sockaddr*>(&senderAddr_),
		sizeof(senderAddr_));

	if (sent == SOCKET_ERROR) {
		std::cerr << "[UdpReceiver] Ack sendto failed: " << WSAGetLastError() << "\n";
	}
	else {
		std::cout << "[UdpReceiver] Sent ModeChangeAck to client\n";
	}
}

bool TCC::UdpReceiver::parseModeChangeMSG(const char* buffer, ModeChangeMSG& msg) {
	memcpy(&msg.mode_, buffer, sizeof(unsigned int));
	if (msg.mode_ != 0 && msg.mode_ != 1)
		return false;
	return true;
}

bool TCC::UdpReceiver::parseManualFireMSG(const char* buffer, ManualFireMSG& msg) {
	memcpy(&msg, buffer, sizeof(ManualFireMSG));

	std::cout << std::string(msg.commandId_, 20) << std::endl;
	std::cout << std::string(msg.targetAircraftId_, 8) << std::endl;

	//if (!TCC::isValidCommandId(msg.commandId_))
	//	return false;
	//if (!TCC::isValidAircraftId(msg.targetAircraftId_))
	//	return false;

	return true;
}

bool TCC::UdpReceiver::parseEmergencyDestroyMSG(const char* buffer, EmergencyDestroyMSG& msg) {
	memcpy(&msg, buffer, sizeof(EmergencyDestroyMSG));

	std::cout << std::string(msg.commandId_, 20) << std::endl;
	std::cout << std::string(msg.targetMissileId_, 8) << std::endl;

	//if (!TCC::isValidCommandId(msg.commandId_))
	//	return false;
	//if (!TCC::isValidMissileId(msg.targetMissileId_))
	//	return false;

	return true;
}

void TCC::UdpReceiver::parseHeader(const char* buffer, Header& header) {
	memcpy(&header.commandCode_, (unsigned int*)buffer, sizeof(unsigned int));
	memcpy(&header.bodyLength_, (int*)buffer, sizeof(int));
}