
#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

namespace TCC {

	class UdpMulticastReceiver {
	public:
		UdpMulticastReceiver(std::string& multicastIp, int port);

		bool init();

		virtual void receiveLoop() = 0;
		virtual void start() = 0;

	protected:
		std::string multicastIp_;
		int port_;
		SOCKET serverSocket_;
	};
}
