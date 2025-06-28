// InterceptResultPacket.h
#pragma once

#pragma pack(push, 1)
struct InterceptResultPacket {
    unsigned int EventCode;      // ��: 2003
    unsigned int BodyLength;     // payload ����
    char EventId[20];            // ��: "SUCCESS"
    char AirCraftId[8];          // �װ��� ID
    char MissileId[8];           // �̻��� ID
};
#pragma pack(pop)
