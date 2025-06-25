#include "UdpSender.h"
#include "UdpMulticastReceiver.h"

TCC::UdpSender::UdpSender(const std::string& ip, int port)
    : ip_(ip), port_(port), sock_(INVALID_SOCKET) {
    ZeroMemory(&targetAddr_, sizeof(targetAddr_));
    std::cout << "UdpSender created\n";
}

TCC::UdpSender::~UdpSender() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
    }
    WSACleanup();
}

bool TCC::UdpSender::init() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

    targetAddr_.sin_family = AF_INET;
    targetAddr_.sin_port = htons(9001);
    //targetAddr_.sin_addr.s_addr = inet_addr("192.168.2.99"); // occ�� ���� �ּ� - �¿�
    //targetAddr_.sin_addr.s_addr = inet_addr("192.168.2.194"); // occ�� ���� �ּ� - ����
    targetAddr_.sin_addr.s_addr = inet_addr("192.168.2.195"); // occ�� ���� �ּ� - ����
    return true;
}

int TCC::UdpSender::sendByteData(const char* data, int length) {

	if (sock_ == INVALID_SOCKET) {
		std::cerr << "Socket is not initialized\n";
		return -1;
	}

    int bytesSent = sendto(sock_, data, length, 0, (sockaddr*)&targetAddr_, sizeof(targetAddr_));
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "sendto() failed: " << WSAGetLastError() << "\n";
        return -1;
    }
    return bytesSent;
}

bool TCC::UdpSender::sendAircraftData(AircraftManager::NewAircraftWithIP& data){

    char buffer[100];
    //��� ���̱�
    int headerSize = serializeHeader(buffer, CommandCode::AircraftData, 64);
    int bodySize = serializeAircraftSender(buffer+headerSize, data);

    //if (data.aircraftData_.aircraftId_ == "ATS-0001") {
    //    auto now = std::chrono::system_clock::now();
    //    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    //    std::cout << "sendAircraftData() [ATS-0001] timestamp(ms since epoch): " << ms << std::endl;
    //}

    if (sendByteData(buffer, headerSize + bodySize) < 0) {
        return false;
    }
    return true;
}

const int TCC::UdpSender::serializeHeader(char* buffer, const int commandCode, int bodyLength) {
    memcpy(buffer, &commandCode, sizeof(unsigned int));
    memcpy(buffer + 4, &bodyLength, sizeof(int));
    return 8;
}

const int TCC::UdpSender::serializeAircraftSender(char* buffer, AircraftManager::NewAircraftWithIP& data){
    std::memset(buffer + 0, 0, 8);
    std::memcpy(buffer + 0, data.aircraftData_.aircraftId_.c_str(), 8);

    std::memcpy(buffer + 8, &data.aircraftData_.location_, sizeof(double) * 3);

    std::memcpy(buffer + 32, &data.aircraftData_.isEnemy_, sizeof(unsigned int));

    if (data.aircraftData_.isEnemy_) {
        std::memcpy(buffer + 36, &data.engagementStatus_, sizeof(unsigned int));
    }
    else {
        unsigned int engagementStatus = 0;
        std::memcpy(buffer + 36, &engagementStatus, sizeof(unsigned int));
    }

    std::memcpy(buffer + 40, &data.impactPoint_, sizeof(double)*3);

    return 64; // �� ����ȭ�� body����Ʈ ��
}

bool TCC::UdpSender::sendLaunchCommand(std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint) {

    char* buffer = new char[100];
    //��� ���̱�
    //commandCode 201, bodyLength 60
    int headerSize = serializeHeader(buffer, CommandCode::LaunchCommand, 60);
    int bodySize = serializeLaunchCommandBody(buffer + headerSize, commandId, aircraftId, missileId, impactPoint);

    //ACK�����ϱ�..
    if (sendByteData(buffer, headerSize + bodySize) < 0) {
        return false;
    }

    return true;

}

const int TCC::UdpSender::serializeLaunchCommandBody(char* buffer, std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint) {
	std::memset(buffer + 0, 0, 20);
	std::memcpy(buffer + 0, commandId.c_str(), 20);
	std::memcpy(buffer + 20, aircraftId.c_str(), 8);
	std::memcpy(buffer + 28, missileId.c_str(), 8);
	std::memcpy(buffer + 36, &impactPoint, sizeof(TCC::Position));
	return 60; // �� ����ȭ�� body����Ʈ ��
}

bool TCC::UdpSender::sendMissileData(UdpMulticastReceiver::MissileMSG& data) {

    char buffer[44];
    //��� ���̱�
    int headerSize = serializeHeader(buffer, CommandCode::MissileData, 36);
    int bodySize = serializeMissileSender(buffer + headerSize, data);

    if (sendByteData(buffer, headerSize + bodySize) < 0) {
		std::cerr << "sendByteData() failed: " << WSAGetLastError() << "\n";
        return false;
    }

    //std::cout << u8"--------------�̻��� ������ �۽�-----------" << "\n";
    //std::cout << u8"ID : " << data.missileId << "\n";
    //std::cout << u8"Status : " << data.status_ << "\n";
    //std::cout << u8"���� : " << data.location_.latitude_ << "\n";
    //std::cout << u8"�浵 : " << data.location_.longitude_ << "\n";
    //std::cout << u8"�� : " << data.location_.altitude_ << "\n";
    return true;
}


const int TCC::UdpSender::serializeMissileSender(char* buffer, UdpMulticastReceiver::MissileMSG& data) {
    std::memset(buffer + 0, 0, 8);
    std::memcpy(buffer + 0, data.missileId, 8);
    std::memcpy(buffer + 8, &data.status_, sizeof(unsigned int));
    std::memcpy(buffer + 12, &data.location_, sizeof(double)*3);
   
    return 36; // �� ����ȭ�� body����Ʈ ��
}

bool TCC::UdpSender::sendDestroyCommand(std::string&commandId, std::string& aircraftId, std::string& missileId, unsigned int type) {

    char buffer[44];
    //��� ���̱�
    int headerSize = serializeHeader(buffer, CommandCode::DestroyCommand, 36);
    int bodySize = serializeDestroyCommand(buffer + headerSize, commandId, aircraftId, missileId, type);

    if (sendByteData(buffer, headerSize + bodySize) < 0) {
        std::cerr << "sendByteData() failed: " << WSAGetLastError() << "\n";
        return false;
    }

    std::cout << u8"--------------���� �޽��� �۽�-----------" << std::endl;
    std::cout << "commandId: " << commandId << " aircraftId: " << aircraftId << " missileId: " << missileId << std::endl;
    std::cout << u8"-----------------------------------------" << std::endl;
    return true;
}

const int TCC::UdpSender::serializeDestroyCommand(char* buffer, std::string&commandId, std::string& aircraftId, std::string missileId, unsigned int type) {
    std::memset(buffer + 0, 0, 8);
    std::memcpy(buffer + 0, commandId.c_str(), 20);
    std::memcpy(buffer + 20, aircraftId.c_str(), 8);
    std::memcpy(buffer + 28, missileId.c_str(), 8);
    std::memcpy(buffer + 36, &type, sizeof(unsigned int));

    return 40; // �� ����ȭ�� body����Ʈ ��
}

//const int TCC::UdpSender::serializeEmergencySender(char* buffer, std::string commandId, std::string missileId) {
//    std::memset(buffer + 0, 0, 20);
//    std::memcpy(buffer + 0, &commandId, 20);
//    std::memcpy(buffer + 20, &missileId, 8);
//
//    return 28; // �� ����ȭ�� body����Ʈ ��
//}

//bool TCC::UdpSender::sendEmergencyDestroy(std::string commandId, std::string missileId) {
//
//    //������ ���� ���� 1�ʵ��� 10�� �۽�
//    //ack���� �Ҷ�����
//
//    char buffer[36];
//    //��� ���̱�
//    int headerSize = serializeHeader(buffer, 202, 28);
//    int bodySize = serializeEmergencySender(buffer + headerSize, commandId, missileId);
//
//    // �۽� ������ ����
//    std::thread([=]() {
//        for (int i = 0; i < 10; ++i) {
//            sendByteData(buffer, headerSize + bodySize);
//            std::this_thread::sleep_for(std::chrono::milliseconds(100));
//			// Todo : ack ���� ��� ���� �߰� �ʿ�
//        }
//    }).detach();
//
//    // ȣ�� ��� true ��ȯ (�����忡�� ���� �۽�)
    //return true;

    /*if (sendByteData(buffer, headerSize + bodySize) < 0) {
        return false;
    }
    return true;*/
//}


