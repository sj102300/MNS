#include "ShootDownCheck.h"
#include <iostream>

void ShootDownCheck::initializeMultiSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return;
    }

    udpSocket2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket2 == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    multicastAddr2.sin_family = AF_INET;
    if (inet_pton(AF_INET, "239.0.0.2", &multicastAddr2.sin_addr) <= 0) {
        std::cerr << "Invalid multicast address" << std::endl;
        closesocket(udpSocket2);
        WSACleanup();
        return;
    }
    multicastAddr2.sin_port = htons(9999);

    int ttl = 1;
    if (setsockopt(udpSocket2, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << "Failed to set socket options. Error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket2);
        WSACleanup();
        return;
    }
}

void ShootDownCheck::sendSuccessInfo(std::string id, std::string missileID) {
    std::cout << "격추성공: 항공기 ID " << id << ", 미사일 ID " << missileID << std::endl;
	char buffer[(sizeof(char) * 8) * 2 + sizeof(unsigned int)];
	{
		//lock_guard<mutex> lock(mtx);
		// double 값을 비트스트림으로 변환
		unsigned int success = 2003;

		memcpy(buffer, id.c_str(), 8);
		memcpy(buffer + 8, missileID.c_str(), 8);
		memcpy(buffer + 16, &success, sizeof(unsigned int));
	}

	//sendto 함수를 통해 통신(송신)
	int sendSize = sendto(udpSocket2, buffer, sizeof(buffer), 0, (SOCKADDR*)&multicastAddr2, sizeof(multicastAddr2));
	if (sendSize < 0) {
		std::cerr << "Failed to send data. Error: " << WSAGetLastError() << std::endl;
	}
	std::cout << "통신 성공" << std::endl;
}