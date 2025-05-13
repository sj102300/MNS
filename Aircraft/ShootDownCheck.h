#pragma once
#include "ShootDownChecker.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>

class ShootDownCheck : public ShootDownChecker {
private:

public:
	ShootDownCheck();
	void ShootDown();
	void sendSuccessInfo();
	void MissileReciever();

	void initializeMultiSocket(SOCKET& udpSocket, sockaddr_in& multicastAddr);
};