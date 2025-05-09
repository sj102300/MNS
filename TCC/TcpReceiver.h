#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

namespace TCC {
    template<typename T>
    class TcpReceiver {
    public:
        TcpReceiver(std::string ip_address, int port) : ip_address_(ip_address), port_(port) {
            init();
        }

        // 초기화: 소켓 생성, 바인딩 등
		bool init() {

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
			serverAddr_.sin_port = htons(port_);	//포트를 네트워크 바이트 순서로 변환
			inet_pton(AF_INET, ip_address_.c_str(), &serverAddr_.sin_addr);	//문자열 ip를 이진형으로 변환

			if (bind(serverSocket_, (sockaddr*)&serverAddr_, sizeof(serverAddr_)) == SOCKET_ERROR) {
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

        // 메시지 파싱
        virtual void parseMsg(T &msg, const char* buffer, int length) = 0;

        // 클라이언트 연결 수락
		bool connectClient() {
			clientSocket_ = accept(serverSocket_, (sockaddr*)&clientAddr_, &clientAddrLen_);
			if (clientSocket_ == INVALID_SOCKET) {
				std::cerr << "Client Accept Failed\n";
				return false;
			}

			//필요하면 clientAddr_, clientAddrLen_으로 클라이언트 주소 구하기 가능
			std::cout << "Client Connected\n";
			return true;
		}

        virtual void receive() = 0;

        // 메인에서 한번만 호출. 스레드로 만들어서 대기
        virtual void start() = 0;

        // 응답 처리
        virtual void response() = 0;

        // 소켓 핸들링 관련 로직
        //virtual void getSocket();

    protected:
        SOCKET serverSocket_;
        SOCKET clientSocket_;
        sockaddr_in serverAddr_;
        sockaddr_in clientAddr_;
        int clientAddrLen_;

        std::string ip_address_;
        int port_;
    };
}