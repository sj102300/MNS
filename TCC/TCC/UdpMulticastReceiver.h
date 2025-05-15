
#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>  // 꼭 추가 필요
#include <iostream>
#include <thread>

namespace TCC {
	class UdpMulticastReceiver {
	public:
		UdpMulticastReceiver(const std::string& multicastIp, int port);

		bool init();
		virtual void receive() = 0;

	protected:
		std::string multicastIp_;
		int port_;
		SOCKET serverSocket_;
	};
}
