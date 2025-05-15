#include "LaunchCommandSender.h"
#pragma warning(disable: 4996)

LaunchCommandSender::LaunchCommandSender(std::string ip_address, int port) : TcpSender(ip_address, port) {

}

void makePacket(char* buffer, const std::string& fireCommand, const std::string& aircraftId) {
    // ���� �ʱ�ȭ
    memset(buffer, 0, 28);

    // �߻��� �ĺ��� (20 bytes)
    strncpy(buffer, fireCommand.c_str(), 20);

    // �װ��� �ĺ��� (8 bytes)
    strncpy(buffer + 20, aircraftId.c_str(), 8);

    // ������ ������ Ȯ��
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

