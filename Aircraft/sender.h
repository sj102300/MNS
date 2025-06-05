#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

// 외부에서 사용하기 위한 소켓, 주소 구조체 선언
extern SOCKET udpSocket2;
extern sockaddr_in multicastAddr2;

void initializeMultiSenderSocket();
void sendSuccessInfo(std::string id, std::string missileID);

// ?? 구조체 정의 추가 (패딩 방지)
#pragma pack(push, 1)
struct InterceptResultPacket {
    unsigned int EventCode;      // 예: 2003
    unsigned int BodyLength;     // payload 길이
    char EventId[20];            // 예: "SUCCESS"
    char MissileId[8];           // 미사일 ID
    char AirCraftId[8];          // 항공기 ID
};
#pragma pack(pop)
