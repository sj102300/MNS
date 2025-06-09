#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UdpReceiver.h"
#include "Packet.h"


UdpReceiver::UdpReceiver()
    : sock_(INVALID_SOCKET), running_(false), port_(0) {
}

bool UdpReceiver::init(const std::string& multicast_address, int port) {
    port_ = port;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        return false;
    }

    // ���� �����ϵ��� ����
    int reuse = 1;
    if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt() failed\n";
        return false;
    }

    // ���� �ּ� ����
    memset(&localAddr_, 0, sizeof(localAddr_));
    localAddr_.sin_family = AF_INET;
    localAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr_.sin_port = htons(port_);

    if (bind(sock_, (struct sockaddr*)&localAddr_, sizeof(localAddr_)) < 0) {
        std::cerr << "bind() failed\n";
        return false;
    }

    // ��Ƽĳ��Ʈ �׷� ����
    mreq_.imr_multiaddr.s_addr = inet_addr(multicast_address.c_str());
    mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_)) < 0) {
        std::cerr << "setsockopt(IP_ADD_MEMBERSHIP) failed\n";
        return false;
    }

    return true;
}

void UdpReceiver::start() {
    running_ = true;
    recvThread_ = std::thread(&UdpReceiver::run, this);
}

void UdpReceiver::run() {
    char buffer[2048];  // ����� �˳��� ũ���
    while (running_) {
        sockaddr_in senderAddr;
        int senderLen = sizeof(senderAddr);
        int bytesReceived = recvfrom(sock_, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderLen);

        if (bytesReceived > 8) {
            // 1. eventCode�� bodyLength �Ľ�
            uint32_t eventCode;
            uint32_t bodyLength;

            memcpy(&eventCode, buffer, sizeof(unsigned int));
            memcpy(&bodyLength, buffer + 4, sizeof(unsigned int));

            if (bytesReceived < 8 + static_cast<int>(bodyLength)) {
                std::cerr << u8"�߸��� ��Ŷ ����\n";
                continue;
            }

            //const char* bodyPtr = buffer + 8;
            const char* bodyPtr = buffer;

            switch (eventCode) {
            case 2002: { // �̻��� �߻� ��� ó��
                if (bodyLength < sizeof(OrderPacket)-8) {
                    std::cerr << u8"�̻��� ��Ŷ ���� ����\n";
                    std::cout << u8"packet size : " << sizeof(OrderPacket) << "\n";
                    break;
                }

                OrderPacket orderpkg;
                memcpy(&orderpkg, bodyPtr, sizeof(orderpkg));
                std::string missileId(orderpkg.MissileId, strnlen(orderpkg.MissileId, 8));
                Location impactPoint = orderpkg.ImpactPoint;

                std::cout << u8"������ �̻��� ID: [" << missileId << "]\n";

                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    std::cout << u8" �̻��� ������Ʈ �ϴ��� \n";
                    auto missile = it->second;
                    missile->setMissileId(missileId);
                    missile->setState(1);  // ��: 3 = �߻� �� ���� ����
                    missile->setTargetLocation(impactPoint);
                }
                std::cout << u8"[������Ʈ��] �̻���: " << missileId
                    << u8" �� ��ǥ: (" << impactPoint.altitude << ", " << impactPoint.latitude << ")\n";
              
                break;
            }

            case 2003: { // ���� ���� 
                if (bodyLength < sizeof(InterceptResultPacket) - 8) {
                    std::cerr << u8"�̻��� InterceptResultPacket ��Ŷ ���� ����\n";
                    std::cout << u8"packet size : " << sizeof(InterceptResultPacket) << "\n";
                    break;
                }

                InterceptResultPacket Resultpkg;
                memcpy(&Resultpkg, bodyPtr, sizeof(Resultpkg));

                std::string missileId(Resultpkg.MissileId, strnlen(Resultpkg.MissileId, 8));

                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    std::cout << u8" �̻��� ������Ʈ �ϴ��� \n";
                    auto missile = it->second;
                    missile->setMissileId(missileId);
                    missile->setState(2);  // 2���� ���� ����!!
                }
                std::cout << u8"[������Ʈ��] �̻���: " << missileId << u8"���� ���·� ��ȯ\n";

                break;
            }
            case 2004: { // ��� ���� ��� 
                if (bodyLength < sizeof(EDPacket) - 8) {
                    std::cerr << u8"�̻��� ED ��Ŷ ���� ����\n";
                    std::cout << u8"packet size : " << sizeof(EDPacket) << "\n";
                    break;
                }

                EDPacket EDpkg;
                memcpy(&EDpkg, bodyPtr, sizeof(EDpkg));

                std::string missileId(EDpkg.MissileId, strnlen(EDpkg.MissileId, 8));
                std::cout << u8"������ �̻��� ID: [" << missileId << "]\n";

                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    std::cout << u8" �̻��� ������Ʈ �ϴ��� \n";
                    auto missile = it->second;
                    missile->setMissileId(missileId);
                    missile->setState(3);  // 3���� ������� ������
                }
                std::cout << u8"[������Ʈ��] �̻���: " << missileId << "������� ���·� ��ȯ\n";

                break;
            }
            default:
                //std::cerr << u8"�� �� ���� �̺�Ʈ �ڵ�: " << eventCode << "\n";
                break;
            }
        }
    }
}

void UdpReceiver::close() {
    running_ = false;
    if (recvThread_.joinable()) {
        recvThread_.join();
    }

    // ��Ƽĳ��Ʈ �׷� Ż��
    setsockopt(sock_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_));
    closesocket(sock_);
    WSACleanup();
}

void UdpReceiver::setMissileMap(const std::unordered_map<std::string, std::shared_ptr<Missile>>& map) {
    missile_map_ = map;
}
