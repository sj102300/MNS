#pragma once

#include "UdpReceiver.h"
#include "UdpMulSender.h"

class LCHLauncher {
public:
    LCHLauncher();
    ~LCHLauncher();

    bool init(const std::string& recvAddr, int recvPort,const std::string& sendAddr, int sendPort);

    void run();

    void stop();

    bool isRunning();

private:
    std::unique_ptr<UdpReceiver> receiver_;
    std::unique_ptr<UdpMulSender> sender_;
    std::atomic<bool> running_;
};
