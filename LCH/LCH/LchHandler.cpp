#if 1
#pragma once
#include "LchHandler.h"
#include <iostream>
#include <cstring>  // memcpy

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
        int bytes = receiver_->receive(buffer, sizeof(buffer));
        if (bytes < 8) continue;

        uint32_t eventCode;
        uint32_t bodyLength;
        memcpy(&eventCode, buffer, 4);
        memcpy(&bodyLength, buffer + 4, 4);

        if (bytes < 8 + static_cast<int>(bodyLength)) {
            std::cerr << "[LCH] 원하는 정보가 아님\n";
            continue;
        }

        if (eventCode == 2001) {
            std::cout << "[LCH] 발사명령 수신 → MSS로 전송 중...\n";

            // eventCode 수정: 2001 → 2002
            uint32_t newEventCode = 2002;
            memcpy(buffer, &newEventCode, sizeof(uint32_t));

            int sent = sender_->send(buffer, bytes);
            if (sent < 0)
                std::cerr << "[LCH] MSS 전송 실패\n";
            else
                std::cout << "[LCH] MSS 전송 완료 (" << sent << " bytes)\n";
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

