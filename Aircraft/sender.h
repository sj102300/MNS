#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

// �ܺο��� ����ϱ� ���� ����, �ּ� ����ü ����
extern SOCKET udpSocket2;
extern sockaddr_in multicastAddr2;

void initializeMultiSenderSocket();
void sendSuccessInfo(std::string id, std::string missileID);

// ?? ����ü ���� �߰� (�е� ����)
#pragma pack(push, 1)
struct InterceptResultPacket {
    unsigned int EventCode;      // ��: 2003
    unsigned int BodyLength;     // payload ����
    char EventId[20];            // ��: "SUCCESS"
    char MissileId[8];           // �̻��� ID
    char AirCraftId[8];          // �װ��� ID
};
#pragma pack(pop)
