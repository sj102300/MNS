#include "LaunchCommandSender.h"
#pragma warning(disable: 4996)

LaunchCommandSender::LaunchCommandSender(std::string ip_address, int port) : TcpSender(ip_address, port) {

}

void makePacket(char* buffer, const std::string& fireCommand, const std::string& aircraftId) {
    // 버퍼 초기화
    memset(buffer, 0, 28);

    // 발사명령 식별자 (20 bytes)
    strncpy(buffer, fireCommand.c_str(), 20);

    // 항공기 식별자 (8 bytes)
    strncpy(buffer + 20, aircraftId.c_str(), 8);

    // 디버깅용 데이터 확인
    std::cout << "Packet Data: ";
    for (int i = 0; i < 28; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
}


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

