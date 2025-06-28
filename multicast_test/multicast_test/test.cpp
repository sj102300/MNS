#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define MULTICAST_GROUP "239.0.0.1"
#define MULTICAST_PORT 9001

int main() {
    // WinSock �ʱ�ȭ
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    // UDP ���� ����
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }

    // ���ſ� ���ε� (INADDR_ANY �Ǵ� Ư�� IP�� ���� ����)
    sockaddr_in recvAddr = {};
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // �Ǵ� inet_addr("192.168.15.8")
    recvAddr.sin_port = htons(0);  // ���� ��Ʈ (�������� �ʾƵ� ����)
    if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // ��Ƽĳ��Ʈ ���� �ּ� ����
    sockaddr_in mcastAddr = {};
    mcastAddr.sin_family = AF_INET;
    mcastAddr.sin_port = htons(MULTICAST_PORT);
    mcastAddr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);

    // TTL ����
    int ttl = 4;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));

    // ��Ƽĳ��Ʈ �������̽� ���� (192.168.15.8�� �۽�)
    in_addr localInterface;
    localInterface.s_addr = inet_addr("192.168.15.8");
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localInterface, sizeof(localInterface));

    // ���� ����
    while (true) {
        std::string message = "Hello from client";
        int result = sendto(sock, message.c_str(), message.length(), 0,
            (sockaddr*)&mcastAddr, sizeof(mcastAddr));
        if (result == SOCKET_ERROR) {
            std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
        }
        else {
            std::cout << "Sent multicast message: " << message << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
