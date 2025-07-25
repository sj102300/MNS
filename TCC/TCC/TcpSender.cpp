#include "TcpSender.h"
#pragma comment(lib, "ws2_32.lib")

TCC::TcpSender::TcpSender(std::string ip_address, int port)
    : ip_address_(ip_address),
    port_(port),
    serverSocket_(INVALID_SOCKET),
    clientSocket_(INVALID_SOCKET),
    clientAddrLen_(sizeof(clientAddr_))
{
    std::memset(&clientAddr_, 0, sizeof(clientAddr_));
    std::memset(&serverAddr_, 0, sizeof(serverAddr_));
    std::cout << "TcpSender created\n";
}

bool TCC::TcpSender::init() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    clientSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket_ == INVALID_SOCKET) {
        std::cerr << "Socket Creation Failed\n";
        return false;
    }

	serverAddr_.sin_family = AF_INET; //IpV4
	serverAddr_.sin_port = htons(port_); //포트를 네트워크 바이트 순서로 변환
    inet_pton(AF_INET, ip_address_.c_str(), &serverAddr_.sin_addr); //문자열 ip를 이진형으로 변환

    return true;
}

bool TCC::TcpSender::connectToServer() {
    if (connect(clientSocket_, (sockaddr*)&serverAddr_, sizeof(serverAddr_)) == SOCKET_ERROR) {
        std::cerr << "Connection to Server Failed\n";
        return false;
    }

    std::cout << "Connected to Server\n";
    return true;
}

bool TCC::TcpSender::sendByteData(const char* data, int length) {
    int bytesSent = send(clientSocket_, data, length, 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "send() Failed\n";
        return false;
    }

    return true;
}

void TCC::TcpSender::disconnect() {
    closesocket(clientSocket_);
    WSACleanup();
    std::cout << "Disconnected from Server\n";
}