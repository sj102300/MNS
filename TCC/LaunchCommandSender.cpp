#include "LaunchCommandSender.h"

LaunchCommandSender::LaunchCommandSender(std::string ip_address, int port) : TcpSender(ip_address, port) {

}

//void LaunchCommandSender::makePacket(LaunchCommandPacket& packet, const char* buffer, int length) {
//	// ��Ŷ ����
//}

void LaunchCommandSender::sendLoop() {

    char buffer[28];
    int len = 28;

    while (true) {
        //���� ����� ��Ծƴ°�
        //������ �о����
        //LaunchCommandPacket packet = getData();
        //makePacket(packet, buffer);
        //sendData(buffer, len);
        //�ٽ� ����...
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

