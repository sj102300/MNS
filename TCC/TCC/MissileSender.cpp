#include "MissileSender.h"
#include "UdpSender.h"
#include <iostream>

void MissileSender::sendMissileData() {
    std::cout << "[MissileSender] Sending missile data...\n";
    makeMissilePacket();
    //udpSender->sendData();
}

void MissileSender::makeMissilePacket() {
    std::cout << "[MissileSender] Making missile packet...\n";
}
