#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
struct Location {
    double x;
    double y;
    double z;
};

struct OrderPacket {
    unsigned int EventCode;    // ��: 1 = �߻� ���
    unsigned int BodyLength;   // sizeof(OrderPacket) - 8
    char EventId[20];
    char AtsId[8];
    char MissileId[8];
    Location ImpactPoint;
};
#pragma pack(pop)


#pragma pack(push,1)
struct EDPacket {
    unsigned int EventCode;
    unsigned int BodyLength;
    char EventId[20];
    char MissileId[8];
};
#pragma pack(pop)

#pragma pack(push,1)
struct InterceptResultPacket {
    unsigned int EvenetCode;
    unsigned int BodyLength;
    char EventId[20];
    char MissileId[8];
    char AirCraftId[8];
};
#pragma pack(pop)
int main() {
    // 1. ���� �ʱ�ȭ
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup ����\n";
        return -1;
    }

    // 2. ���� ����
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "���� ���� ����\n";
        WSACleanup();
        return -1;
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        std::cerr << u8"[MFR] setsockopt(SO_REUSEADDR) ����: " << WSAGetLastError() << "\n";
        return false;
    }


    int ttl = 5;  // ���� ����ݱ����� ����
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
        std::cerr << "setsockopt(IP_MULTICAST_TTL) ����\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }        // ��Ƽĳ��Ʈ �׷� ����
            // ��Ƽĳ��Ʈ �۽� �������̽� ����

    in_addr localInterface{};
    if (InetPton(AF_INET, L"192.168.2.22", &localInterface) != 1) {
        std::cerr << "[MFR] InetPton ����\n";
        return false;
    }
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,
        (char*)&localInterface, sizeof(localInterface)) == SOCKET_ERROR) {
        std::cerr << u8"[MFR] ��Ƽĳ��Ʈ �۽� �������̽� ���� ����: " << WSAGetLastError() << "\n";
        return false;
    }

    // 3. ��Ƽĳ��Ʈ �ּ� ����
    sockaddr_in destAddr{};
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(9000);  // ���� ���� ��Ʈ ��ġ 
    inet_pton(AF_INET, "239.0.0.1", &destAddr.sin_addr);

     //4. ��Ŷ ����
    OrderPacket packet{};
    packet.EventCode = 2001;
    packet.BodyLength = sizeof(OrderPacket) - 8;  // EventCode�� BodyLength ����
    strncpy(packet.EventId, "EVT-Launch-01", sizeof(packet.EventId));
    strncpy(packet.AtsId, "ATS001", sizeof(packet.AtsId));
    strncpy(packet.MissileId, "MSS-101", sizeof(packet.MissileId));
    packet.ImpactPoint = { 22.0, 0.0, 10.0 };

    //EDPacket packet{};
    //packet.EventCode = 2004;
    //packet.BodyLength = sizeof(EDPacket) - 8;  // EventCode�� BodyLength ����
    //strncpy(packet.EventId, "EVT-Launch-01", sizeof(packet.EventId));
    //strncpy(packet.MissileId, "MSS-104", sizeof(packet.MissileId));

    
    //
    //InterceptResultPacket packet{};

    //packet.EvenetCode = 2003;
    //packet.BodyLength = sizeof(Intercept) - 8;  // EventCode�� BodyLength ����
    //strncpy(packet.EventId, "EVT-Launch-01", sizeof(packet.EventId));
    //strncpy(packet.AirCraftId, "ATS001", sizeof(packet.AirCraftId));
    //strncpy(packet.MissileId, "MSS-100", sizeof(packet.MissileId));

        
    
    
    // 5. ������ ����
    int sent = sendto(sock, reinterpret_cast<const char*>(&packet), sizeof(packet), 0,
        reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));
    if (sent == SOCKET_ERROR) {
        std::cerr << "sendto ����: " << WSAGetLastError() << "\n";
    }
    else {
        std::cout << "OrderPacket ���� �Ϸ� (" << sent << " bytes)\n";
    }

    // 6. ����
    closesocket(sock);
    WSACleanup();
    return 0;
}
