
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UdpMulticastReceiver.h"

TCC::UdpMulticastReceiver::UdpMulticastReceiver(const std::string& multicastIp, int port) 
	: multicastIp_(multicastIp), port_(port), serverSocket_(INVALID_SOCKET), aircraftManager_(nullptr)
{
	//여기서도 명시적으로 nullptr 넣어주기
	std::cout << "UdpMulticastReceiver created\n";
}

bool TCC::UdpMulticastReceiver::init(AircraftManager* aircraftManager) {

	//여기서 포인터 연결 먼저 하기
	if (aircraftManager == nullptr) {
		return false;
	}
	aircraftManager_ = aircraftManager;

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
	localAddr.sin_port = htons(port_);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(serverSocket_, (sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
		std::cout << "Bind Failed\n";
		return false;
	}

	ip_mreq mreq = {};
	mreq.imr_multiaddr.s_addr = inet_addr(multicastIp_.c_str());
	mreq.imr_interface.s_addr = inet_addr("127.0.0.1"); // ← 수신에 사용할 NIC의 실제 IP로 지정

	if (setsockopt(serverSocket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
		std::cout << "setsockopt(IPP_ADD_MEMBERSHIP) Failed\n";
		return false;
	}
	return true;
}

void TCC::UdpMulticastReceiver::start() {
	recvThread_ = std::thread(&TCC::UdpMulticastReceiver::receive, this);
}

void TCC::UdpMulticastReceiver::receive() {

	Header header;
	sockaddr_in senderAddr;
	int addrLen = sizeof(senderAddr);
	AircraftManager::NewAircraft newAircraft;

	while (true) {
		int bytesReceived = recvfrom(serverSocket_, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr, &addrLen);
		
		if (bytesReceived < 0) {
			std::cerr << "recvfrom Failed. Error: " << WSAGetLastError() << "\n";
			break;
		}

		parseHeader(header);
		std::cout << header.eventCode_ << std::endl;

		switch (header.eventCode_) {
		case 1001:
			//ATS->MFR로 보내는데이터임 할일없음
			break;
		case 1002:
			//MFR에서 보내는 데이터임
			if (!parseReceivedAircraftMSG(buffer + 8, newAircraft, header.bodyLength_))
				break;
			aircraftManager_->handleReceivedAircraft(newAircraft);
			break;
		case 2001:
			//발사 명령 요청
			break;
		case 2002:
			//발사 수행 요청
			break;
		case 2003:
			//격추 성공
		case 2004:
			//비상 폭파
		case 3001:
			//미사일 데이터
			//여기서 호출
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

	if (!TCC::isValidAircraftId(msg.aircraftId_))
		return false;

	if (!msg.location_.isValidPosition()) 		
		return false;

	if (msg.ourOrEnemy_ != 'E' && msg.ourOrEnemy_ != 'O')
		return false;

	newAircraft.aircraftId_ = std::string(msg.aircraftId_, 8);
	newAircraft.isEnemy_ = msg.ourOrEnemy_ == 'E';
	newAircraft.location_ = msg.location_;

	return true;
}
