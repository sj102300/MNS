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
    targetAddr_.sin_port = htons(port_);
    targetAddr_.sin_addr.s_addr = inet_addr(ip_.c_str());

    return true;
}

int TCC::UdpSender::sendByteData(const char* data, int length) {
    int bytesSent = sendto(sock_, data, length, 0,
        (sockaddr*)&targetAddr_, sizeof(targetAddr_));
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "sendto() failed: " << WSAGetLastError() << "\n";
        return -1;
    }
    return bytesSent;
}

bool TCC::UdpSender::sendAircraftData(AircraftManager::NewAircraftWithIP& data){

    char buffer[72];
    //헤더 붙이기
    int headerSize = serializeHeader(buffer, 100, 64);
    int bodySize = serializeAircraftSender(buffer+headerSize, data);

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
    std::memcpy(buffer + 36, &data.engagementStatus_, sizeof(unsigned int));

    std::memcpy(buffer + 40, &data.impactPoint_, sizeof(double)*3);

    return 64; // 총 직렬화된 body바이트 수
}

bool TCC::UdpSender::sendMissileData(UdpMulticastReceiver::MissileMSG& data) {

    char buffer[44];
    //헤더 붙이기
    int headerSize = serializeHeader(buffer, 300, 36);
    int bodySize = serializeMissileSender(buffer + headerSize, data);

    if (sendByteData(buffer, headerSize + bodySize) < 0) {
		std::cerr << "sendByteData() failed: " << WSAGetLastError() << "\n";
        return false;
    }

    std::cout << u8"--------------미사일 데이터 송신-----------" << "\n";
    std::cout << u8"ID : " << data.missileId << "\n";
    std::cout << u8"Status : " << data.status_ << "\n";
    std::cout << u8"위도 : " << data.location_.latitude_ << "\n";
    std::cout << u8"경도 : " << data.location_.longitude_ << "\n";
    std::cout << u8"고도 : " << data.location_.altitude_ << "\n";
    return true;
}


const int TCC::UdpSender::serializeMissileSender(char* buffer, UdpMulticastReceiver::MissileMSG& data) {
    std::memset(buffer + 0, 0, 8);
    std::memcpy(buffer + 0, data.missileId, 8);
    std::memcpy(buffer + 8, &data.status_, sizeof(unsigned int));
    std::memcpy(buffer + 12, &data.location_, sizeof(double)*3);
   
    return 36; // 총 직렬화된 body바이트 수
}

const int TCC::UdpSender::serializeEmergencySender(char* buffer, std::string commandId, std::string missileId) {
    std::memset(buffer + 0, 0, 20);
    std::memcpy(buffer + 0, &commandId, 20);

    std::memcpy(buffer + 20, &missileId, 8);

    return 28; // 총 직렬화된 body바이트 수
}

bool TCC::UdpSender::sendEmergencyDestroy(std::string commandId, std::string missileId) {

    //스레드 따로 만들어서 1초동안 10번 송신
    //ack수신 할때까지

    char buffer[36];
    //헤더 붙이기
    int headerSize = serializeHeader(buffer, 202, 28);
    int bodySize = serializeEmergencySender(buffer + headerSize, commandId, missileId);

    // 송신 스레드 생성
    std::thread([=]() {
        for (int i = 0; i < 10; ++i) {
            sendByteData(buffer, headerSize + bodySize);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
			// Todo : ack 수신 대기 로직 추가 필요
        }
    }).detach();

    // 호출 즉시 true 반환 (스레드에서 실제 송신)
    return true;

    /*if (sendByteData(buffer, headerSize + bodySize) < 0) {
        return false;
    }
    return true;*/
}


