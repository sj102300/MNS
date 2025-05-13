#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET recvSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (recvSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(9999);

    if (bind(recvSocket, (struct sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(recvSocket);
        WSACleanup();
        return 1;
    }

    ip_mreq mreq;
    if (inet_pton(AF_INET, "239.0.0.2", &mreq.imr_multiaddr) <= 0) {
        std::cerr << "Invalid multicast address" << std::endl;
        closesocket(recvSocket);
        WSACleanup();
        return 1;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(recvSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
        std::cerr << "Failed to join multicast group. Error: " << WSAGetLastError() << std::endl;
        closesocket(recvSocket);
        WSACleanup();
        return 1;
    }

    char buffer[8 + sizeof(double) * 2];
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    while (true) {
        int bytesReceived = recvfrom(recvSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr, &senderAddrSize);
        if (bytesReceived > 0) {
            // ID 읽기
            char id[9] = { 0 }; // Null-terminate the string
            memcpy(id, buffer, 8);

            // double 값 복원
            double x, y;
            memcpy(&x, buffer + 8, sizeof(double));
            memcpy(&y, buffer + 8 + sizeof(double), sizeof(double));

            // 결과 출력
            std::cout << "Received ID: " << id << ", x: " << x << ", y: " << y << std::endl;
        }
        else {
            std::cerr << "recvfrom failed. Error: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    closesocket(recvSocket);
    WSACleanup();
    return 0;
}
