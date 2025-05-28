
#include "UdpMulticastSender.h"

TCC::UdpMulticastSender::UdpMulticastSender(std::string ip, int port): multicastIP_(ip), port_(port) {

}

bool TCC::UdpMulticastSender::init() {

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    // 소켓 생성 (UDP)
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    destAddr_.sin_family = AF_INET;
    destAddr_.sin_port = htons(port_);


    if (inet_pton(AF_INET, "239.0.0.1", &destAddr_.sin_addr) <= 0) { // destAddr을 이제 멀티캐스트 주소로
        std::cerr << u8"Invalid multicast address format\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    in_addr localInterface{};
    if (InetPton(AF_INET, L"192.168.2.194", &localInterface) != 1) {
        std::cerr << u8"Failed to convert local interface IP\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localInterface, sizeof(localInterface)) == SOCKET_ERROR) {
        std::cerr << u8"setsockopt(IP_MULTICAST_IF) failed: " << WSAGetLastError() << "\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    int ttl = 4;
    if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << u8"setsockopt(IP_MULTICAST_TTL) failed\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    return true;
}

bool TCC::UdpMulticastSender::sendLaunchCommand(std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint) {

    std::cout << "sendLaunchCommand() called\n";

    char buffer[100];
    //헤더 붙이기
    int headerSize = serializeHeader(buffer, 2001, 60);
    int bodySize = serializeLauncCommandBody(buffer + headerSize, commandId, aircraftId, missileId, impactPoint);

    if (sendByteData(buffer, headerSize + bodySize) < 0) {
        return false;
    }
    std::cout << "Launch command sendByteData() success\n";

    return true;
}

const int TCC::UdpMulticastSender::serializeHeader(char* buffer, unsigned int eventCode, int bodyLength) {
    memcpy(buffer, &eventCode, sizeof(unsigned int));
    memcpy(buffer + 4, &bodyLength, sizeof(int));
    return 8;
}

const int TCC::UdpMulticastSender::serializeLauncCommandBody(char* buffer, std::string& commandId, std::string& aircraftId,
    std::string& missileId, TCC::Position& impactPoint) {
    memcpy(buffer, commandId.c_str(), 20);
    memcpy(buffer + 20, aircraftId.c_str(), 8);
    memcpy(buffer + 28, missileId.c_str(), 8);
    memcpy(buffer + 36, &impactPoint, 24);
    return 60;
}


int TCC::UdpMulticastSender::sendByteData(const char* data, int length) {

    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Socket is not initialized\n";
        return -1;
    }

    int bytesSent = sendto(sock_, data, length, 0, (sockaddr*)&destAddr_, sizeof(destAddr_));
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "sendto() failed: " << WSAGetLastError() << "\n";
        return -1;
    }
    return bytesSent;
}
