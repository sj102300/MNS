#include "LaunchCommandSender.h"

LaunchCommandSender::LaunchCommandSender(std::string ip_address, int port) : TcpSender(ip_address, port) {

}

//void LaunchCommandSender::makePacket(LaunchCommandPacket& packet, const char* buffer, int length) {
//	// 패킷 생성
//}

void LaunchCommandSender::start() {
    if (!init()) {
        std::cerr << "LaunchCommandSender Init() Failed\n";
        return;
    }

    if (!connectToServer()) {
        std::cerr << "LaunchCommandSender connectToServer() Failed\n";
        return;
    }

    std::thread sendThread([this]() {
        //this->sendData();
        });

    sendThread.join();

}

