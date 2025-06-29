#pragma once
#include "Missile.h"
#include <string>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>

class UdpMulticastReceiver {
public:
    UdpMulticastReceiver();
    ~UdpMulticastReceiver();

    bool init(const std::string& multicast_address, int port);
    void receiveAllMissiles(std::unordered_map<std::string, ParsedMissileData>& missiles);
    
private:
    SOCKET sock_;
    sockaddr_in localAddr_;
    ip_mreq mreq_;
};

void runMissileReceiverThread();