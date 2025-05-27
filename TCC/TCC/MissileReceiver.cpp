// MissileReceiver.cpp
#include "MissileReceiver.h"
#include "UdpMulticastReceiver.h"
#include <iostream>

MissileReceiver::MissileReceiver(const std::string& multicastIp, int port)
	: UdpMulticastReceiver(multicastIp, port) {
	std::cout << "[MissileReceiver] Created\n";
}

//void MissileReceiver::receive() {
//    std::cout << "[MissileReceiver] Receiving missile data...\n";
//    //udpReceiver->receive();
//    //parseMissileMessage();
//}

MissileReceiver::~MissileReceiver()
{
	//if (recvThread_.joinable()) {
	//	recvThread_.join(); // �����ϰ� ���� ���
	//}
	//closesocket(serverSocket_);
}

void MissileReceiver::parseMissileMessage() {
    std::cout << "[MissileReceiver] Parsing missile message...\n";
}
