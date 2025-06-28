//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <iostream>
//
//#pragma comment(lib, "ws2_32.lib")
//
//#define MULTICAST_GROUP "239.0.0.1"
//#define MULTICAST_PORT 9000
//
//int main() {
//    WSADATA wsaData;
//    WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
//
//    sockaddr_in localAddr = {};
//    localAddr.sin_family = AF_INET;
//    localAddr.sin_port = htons(MULTICAST_PORT);
//    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//    bind(sock, (sockaddr*)&localAddr, sizeof(localAddr));
//
//    ip_mreq mreq;
//    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
//    mreq.imr_interface.s_addr = inet_addr("192.168.2.134");  // <- 실제 유선 LAN IP 주소 지정
//    setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
//
//    char buffer[1024];
//    sockaddr_in senderAddr;
//    int senderLen = sizeof(senderAddr);
//
//    while (true) {
//        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&senderAddr, &senderLen);
//        if (len > 0) {
//            buffer[len] = '\0';
//            std::cout << "Received: " << buffer << " from "
//                << inet_ntoa(senderAddr.sin_addr) << ":" << ntohs(senderAddr.sin_port) << std::endl;
//
//            // 유니캐스트로 에코 응답
//            sendto(sock, buffer, len, 0, (sockaddr*)&senderAddr, senderLen);
//        }
//    }
//
//    closesocket(sock);
//    WSACleanup();
//    return 0;
//}
