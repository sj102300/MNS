
#include "TcpReceiver.h"
#pragma comment(lib, "ws2_32.lib")

TCC::TcpReceiver::TcpReceiver(std::string ip_address, int port) : ip_address_(ip_address), port_(port)  {

}

bool TCC::TcpReceiver::init() {
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed\n";
		return false;
	}

	serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket_ == INVALID_SOCKET) {
		std::cerr << "Socket Creation Failed\n";
		return false;
	}

	serverAddr_.sin_family = AF_INET;	//IpV4
	serverAddr_.sin_port = htons(port_);	//��Ʈ�� ��Ʈ��ũ ����Ʈ ������ ��ȯ
	inet_pton(AF_INET, ip_address_.c_str(), &serverAddr_.sin_addr);	//���ڿ� ip�� ���������� ��ȯ

	if (bind(serverSocket_, (sockaddr*)&serverAddr_, sizeof(serverAddr_)) == SOCKET_ERROR){
		std::cerr << "Bind Failed\n";
		return false;
	}

	if (listen(serverSocket_, 1) == SOCKET_ERROR) {
		std::cerr << "Listen Failed\n";
		return false;
	}
	std::cout << "TCP Receiver initialized at " << ip_address_ << ":" << port_ << "\n";
	return true;
}

bool TCC::TcpReceiver::connectClient(){
	clientSocket_ = accept(serverSocket_, (sockaddr*)&clientAddr_, &clientAddrLen_);
	if (clientSocket_ == INVALID_SOCKET) {
		std::cerr << "Client Accept Failed\n";
		return false;
	}
	
	//�ʿ��ϸ� clientAddr_, clientAddrLen_���� Ŭ���̾�Ʈ �ּ� ���ϱ� ����
	std::cout << "Client Connected\n";
	return true;
}