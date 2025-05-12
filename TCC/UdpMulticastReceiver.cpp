
#include "UdpMulticastReceiver.h"

TCC::UdpMulticastReceiver::UdpMulticastReceiver(std::string& multicastIp, int port) : multicastIp_(multicastIp), port_(port) {

}

bool TCC::UdpMulticastReceiver::init() {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup Failed\n";
		return false;
	}
	serverSocket_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket_ == INVALID_SOCKET) {
		std::cerr << "Socket Creation Failed\n";
		return false;
	}

	BOOL reuse = TRUE;
	if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
		std::cerr << "setsocket(SO_REUSEADDR) Failed\n";
		return false;
	}

	sockaddr_in localAddr = {};
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port_);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(serverSocket_, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
		std::cerr << "Bind Failed\n";
		return false;
	}

	ip_mreq mreq = {};
	mreq.imr_multiaddr.s_addr = inet_addr(multicastIp_.c_str());
	mreq.imr_multiaddr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(serverSocket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
		std::cerr << "setsockopt(IPP_ADD_MEMBERSHIP) Failed\n";
		return false;
	}

	return true;
}
