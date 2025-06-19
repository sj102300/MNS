#pragma once

#include "UdpReceiver.h"
#include "UdpMulSender.h"
#include "Packet.h"
#include <vector>

struct MissileStatus {
    std::string missileId;
    unsigned int status;
};

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
    std::vector<MissileStatus> missileStatusList_;
};
