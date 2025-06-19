#if 1
#pragma once
#include "LchHandler.h"
#include <iostream>
#include <cstring>  // memcpy
#include <algorithm> // std::find_if
#include <cstdio>    // printf


LCHLauncher::LCHLauncher():running_(false) {}

LCHLauncher::~LCHLauncher() {
    if (receiver_) receiver_->close();
    if (sender_) sender_->close();
}

bool LCHLauncher::init(const std::string& recvAddr, int recvPort,const std::string& sendAddr, int sendPort) {
    receiver_ = std::make_unique<UdpReceiver>();
    sender_ = std::make_unique<UdpMulSender>();

    if (!receiver_->init(recvAddr, recvPort)) return false;
    if (!sender_->init(sendAddr, sendPort)) return false;

    return true;
}

void LCHLauncher::run() {
    running_ = true;
    char buffer[2048];

    while (running_) {
      
        int bytes = receiver_->receive(buffer, sizeof(buffer)); // 여기서 일단 받는다
       
        if (bytes < 8) continue;

        uint32_t eventCode;
        uint32_t bodyLength;
        char cl[8];
        memcpy(&eventCode, buffer, 4);
        memcpy(&bodyLength, buffer + 4, 4);
        memcpy(&cl, buffer + 44, 8);

        if (bytes < 8 + static_cast<int>(bodyLength)) {
            std::cerr << u8"[LCH] 원하는 정보가 아님\n";
            continue;
        }

        if (eventCode == 2001) {
            std::cout << u8"[LCH] 발사명령 수신 → " << cl <<u8"로 전송 중...\n";

            // eventCode 수정: 2001 → 2002
            uint32_t newEventCode = 2002;
            memcpy(buffer, &newEventCode, sizeof(uint32_t));

            int sent = sender_->send(buffer, bytes);
            if (sent < 0)
                std::cerr << u8"[LCH] MSS 전송 실패\n";
            else
                std::cout << u8"[LCH] MSS 전송 완료 (" << sent << " bytes)\n";
        }else if (eventCode == 3001) {
            if (bodyLength < sizeof(MissilePacket) - 8) {
                std::cerr << u8"[LCH] 미사일 패킷 크기 이상\n";
                continue;
            }

            MissilePacket* pkt = reinterpret_cast<MissilePacket*>(buffer);

            // missileId 문자열 변환 (null 종료 보장 안 될 수 있으니 strnlen 사용)
            std::string missileId(pkt->MissileId, strnlen(pkt->MissileId, sizeof(pkt->MissileId)));
            unsigned int status = pkt->MissileState;

            // 상태 업데이트 또는 새로 추가
            auto it = std::find_if(missileStatusList_.begin(), missileStatusList_.end(),
                [&](const MissileStatus& m) { return m.missileId == missileId; });

            if (it != missileStatusList_.end()) {
                it->status = status;
            }
            else {
                missileStatusList_.push_back({ missileId, status });
            }

           // std::cout << u8"[LCH] 미사일 상태 업데이트 - ID: " << missileId << ", 상태: " << status << "\n";
        }
        
    }
}
void LCHLauncher::stop() {
    running_ = false;
    if (receiver_) receiver_->close();  // close socket → receive() 빠져나오게 유도
    if (sender_) sender_->close();
}

bool LCHLauncher::isRunning() {
    return running_;
}
#endif

