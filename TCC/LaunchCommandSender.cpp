#include "LaunchCommandSender.h"

LaunchCommandSender::LaunchCommandSender(std::string ip_address, int port) : TcpSender(ip_address, port) {

}

//void LaunchCommandSender::makePacket(LaunchCommandPacket& packet, const char* buffer, int length) {
//	// 패킷 생성
//}

void LaunchCommandSender::sendLoop() {

    char buffer[28];
    int len = 28;

    while (true) {
        //언제 깨어날지 어떻게아는가
        //데이터 읽어오고
        //LaunchCommandPacket packet = getData();
        //makePacket(packet, buffer);
        //sendData(buffer, len);
        //다시 잠들기...
    }

}

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
        this->sendLoop();
        });

    sendThread.join();

}

