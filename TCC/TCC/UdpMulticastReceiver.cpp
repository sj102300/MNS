
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UdpMulticastReceiver.h"
#include "MissileManager.h"

#define MULTICAST_GROUP "239.0.0.1"

TCC::UdpMulticastReceiver::UdpMulticastReceiver(const std::string& multicastIp, int port) 
	: multicastIp_(multicastIp), port_(port), serverSocket_(INVALID_SOCKET), aircraftManager_(nullptr)
{
	//여기서도 명시적으로 nullptr 넣어주기
	std::cout << "UdpMulticastReceiver created\n";
}

TCC::UdpMulticastReceiver::~UdpMulticastReceiver() {
	WSACleanup();
}

bool TCC::UdpMulticastReceiver::init(AircraftManager* aircraftManager, MissileManager* missileManager, EngagementManager* engagementManager) {

	isRunning_ = true;
	//여기서 포인터 연결 먼저 하기
	if (aircraftManager == nullptr || missileManager == nullptr || engagementManager == nullptr) {
		return false;
	}

	aircraftManager_ = aircraftManager;
	missileManager_ = missileManager;
	engagementManager_ = engagementManager;

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "WSAStartup Failed\n";
		return false;
	}
	serverSocket_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket_ == INVALID_SOCKET) {
		std::cout << "Socket Creation Failed\n";
		return false;
	}

	BOOL reuse = TRUE;
	if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		std::cout << "setsocket(SO_REUSEADDR) Failed\n";
		return false;
	}

	sockaddr_in localAddr = {};
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(9000);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(serverSocket_, (sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
		std::cout << "Bind Failed\n";
		return false;
	}

	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
	mreq.imr_interface.s_addr = inet_addr("192.168.2.190");  // <- 실제 유선 LAN IP 주소 지정

	if (setsockopt(serverSocket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
		std::cout << "setsockopt(IPP_ADD_MEMBERSHIP) Failed\n";
		return false;
	}
	return true;
}

void TCC::UdpMulticastReceiver::start() {
	isRunning_ = true;
	recvThread_ = std::thread(&TCC::UdpMulticastReceiver::receive, this);
}

void TCC::UdpMulticastReceiver::stop() {
	isRunning_ = false;
	closesocket(serverSocket_);
	if (recvThread_.joinable()) {
		recvThread_.join();
	}
	return;
}

void TCC::UdpMulticastReceiver::receive() {

	Header header;
	int addrLen = sizeof(senderAddr_);
	AircraftManager::NewAircraft newAircraft;
	MissileMSG missileMsg;
	EngagementSuccessMSG engagementSuccessMsg;

	while (isRunning_) {
		int bytesReceived = recvfrom(serverSocket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr_, &addrLen);
		
		if (bytesReceived < 0) {
			std::cout << "UdpMulticastReceiver: recvfrom Failed. Error: " << WSAGetLastError() << "\n";
			break;
		}

		parseHeader(header);

		switch (header.eventCode_) {
		case EventCode::FindTargetEvent:
			if (!parseReceivedAircraftMSG(buffer + 8, newAircraft, header.bodyLength_))
				break;
			aircraftManager_->handleReceivedAircraft(newAircraft);
			break;

		case EventCode::EngagementSuccess:
			if (!parseReceivedEngagementSuccessMSG(buffer + 8, engagementSuccessMsg, header.bodyLength_))
				break;
			responseEngagementSuccessAck(engagementSuccessMsg);
			engagementManager_->engagementSuccess(std::string(engagementSuccessMsg.targetAircraftId_, 8), std::string(engagementSuccessMsg.targetMissileId_, 8));
			break;

		case EventCode::MissileStatus:
			//미사일 데이터
			if (!parseReceivedMissileMSG(buffer + 8, missileMsg, header.bodyLength_))
				break;
			//여기서 호출
			if (missileManager_) {
				missileManager_->echoMissileData(missileMsg);
				//std::cout << u8"---------------미사일 데이터 수신---------------" << "\n";
				//std::cout << u8"ID : " << missileMsg.missileId << "\n";
				//std::cout << u8"Status : " << missileMsg.status_ << "\n";
				//std::cout << u8"위도 : " <<missileMsg.location_.latitude_ << "\n";
				//std::cout << u8"경도 : " << missileMsg.location_.longitude_ << "\n";
				//std::cout << u8"고도 : " << missileMsg.location_.altitude_ << "\n";
			}
			break;
		default:
			break;
		}
	}
}

void TCC::UdpMulticastReceiver::parseHeader(Header& header) {
	header.eventCode_ = *(unsigned int*)(buffer);
	header.bodyLength_ = *(int*)(buffer + 4);
}

bool TCC::UdpMulticastReceiver::parseReceivedAircraftMSG(const char * buffer, AircraftManager::NewAircraft& newAircraft, int length) {

	AircraftMSG msg;
	memcpy((void*)&msg, buffer, length);

	//if (!TCC::isValidAircraftId(msg.aircraftId_))
	//	return false;

	//if (!msg.location_.isValidPosition()) 		
	//	return false;

	//if (msg.ourOrEnemy_ != 'E' && msg.ourOrEnemy_ != 'O')
	//	return false;

	newAircraft.aircraftId_ = std::string(msg.aircraftId_, 8);
	newAircraft.isEnemy_ = msg.ourOrEnemy_ == 'E';
	newAircraft.location_ = msg.location_;

	return true;
}

bool TCC::UdpMulticastReceiver::responseEngagementSuccessAck(EngagementSuccessMSG& msg) {
	Header header;
	header.eventCode_ = EventCode::EngagementSuccess;
	header.bodyLength_ = sizeof(EngagementSuccessMSG);

	char buffer[100];
	memcpy(buffer, &header, sizeof(Header));
	memcpy(buffer + sizeof(Header), &msg, sizeof(EngagementSuccessMSG));
	int sent = sendto(serverSocket_, buffer, sizeof(buffer), 0,
		reinterpret_cast<const sockaddr*>(&senderAddr_),
		sizeof(senderAddr_));

	if (sent == SOCKET_ERROR) {
		std::cerr << "[UdpReceiver] Ack sendto failed: " << WSAGetLastError() << "\n";
		return false;
	}
	else {
		std::cout << "[UdpReceiver] Sent EngagementSuccessAck to client\n";
		return true;
	}
}

bool TCC::UdpMulticastReceiver::parseReceivedMissileMSG(const char* buffer, MissileMSG& data, int length) {

	memcpy((void*)&data, buffer, length);
	/*if (!TCC::isValidMissileId(msg.missileId))
		return false;

	if (!data.location_.isValidPosition())
		return false;*/

	return true;
}

bool TCC::UdpMulticastReceiver::parseReceivedEngagementSuccessMSG(const char* buffer, EngagementSuccessMSG& msg, int length) {
	if (length < sizeof(EngagementSuccessMSG)) {
		std::cout << "EngagementSuccessMSG length error\n";
		return false;
	}
	memcpy(&msg, buffer, sizeof(EngagementSuccessMSG));
	//if (!TCC::isValidAircraftId(msg.targetAircraftId_))
	//	return false;
	//if (!TCC::isValidMissileId(msg.targetMissileId_))
	//	return false;
	return true;
}