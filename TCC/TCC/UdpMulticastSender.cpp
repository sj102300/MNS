
#include "UdpMulticastSender.h"

TCC::UdpMulticastSender::UdpMulticastSender(std::string ip, int port): multicastIP_(ip), port_(port) {

}

bool TCC::UdpMulticastSender::init() {

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    // 소켓 생성 (UDP)
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    destAddr_.sin_family = AF_INET;
    destAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr_.sin_port = htons(9000);
    bind(sock_, (sockaddr*)&destAddr_, sizeof(destAddr_));

    // 멀티캐스트 전송 주소
    sockaddr_in mcastAddr = {};
    mcastAddr.sin_family = AF_INET;
    mcastAddr.sin_port = htons(9000);
    mcastAddr.sin_addr.s_addr = inet_addr("239.0.0.1");

    int ttl = 4;
    setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl));

    if (inet_pton(AF_INET, "239.0.0.1", &destAddr_.sin_addr) <= 0) { // destAddr을 이제 멀티캐스트 주소로
        std::cerr << u8"Invalid multicast address format\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    in_addr localInterface;
    localInterface.s_addr = inet_addr("192.168.2.194"); 
    setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localInterface, sizeof(localInterface));

    if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localInterface, sizeof(localInterface)) == SOCKET_ERROR) {
        std::cerr << u8"setsockopt(IP_MULTICAST_IF) failed: " << WSAGetLastError() << "\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

     //멀티캐스트 전송 시 TTL(Time-To-Live) 옵션 설정 (기본 1: 로컬 네트워크)
    //int ttl = 4;
    //if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)) < 0) {
    //    std::cerr << u8"setsockopt(IP_MULTICAST_TTL) failed\n";
    //    closesocket(sock_);
    //    WSACleanup();
    //    return false;
    //}

    // 수신 타임아웃 설정
    int timeout = 100; // 100ms
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    return true;
}

void TCC::UdpMulticastSender::sendLaunchCommand(std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint) {

	//std::cout << "sendLaunchCommand() called with commandId: " << commandId
	//	<< ", aircraftId: " << aircraftId
	//	<< ", missileId: " << missileId
	//	<< ", impactPoint: (" << impactPoint.latitude_ << ", "
	//	<< impactPoint.longitude_ << ", "
	//	<< impactPoint.altitude_ << ")"
	//	<< std::endl;
    char* buffer = new char[100];
    //헤더 붙이기
    int headerSize = serializeHeader(buffer, EventCode::launchCommand, sizeof(LaunchCommandBody));
    int bodySize = serializeLauncCommandBody(buffer + headerSize, commandId, aircraftId, missileId, impactPoint);
    int totalSize = headerSize + bodySize;

    std::thread([this, buffer, totalSize]() {
        sendUntilReceiveAck(buffer, totalSize);
        delete[] buffer;
        }).detach();
    return;
}

void TCC::UdpMulticastSender::setAckResult(const std::string& missileId, bool result) {
    {
        std::lock_guard<std::mutex> lock(ackMtx_);
        ackResults_[missileId] = result;
    }
    ackCv_.notify_all();
}

bool TCC::UdpMulticastSender::waitForAckResult(const std::string& missileId, int timeoutMs) {
    std::unique_lock<std::mutex> lock(ackMtx_);
    return ackCv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&] {
        return ackResults_.find(missileId) != ackResults_.end();
        }) && ackResults_[missileId];
}

bool TCC::UdpMulticastSender::sendUntilReceiveAck(const char* buffer, int length) {

    char recvBuffer[100];

    sockaddr_in fromAddr;
    int fromLen = sizeof(fromAddr);

    for (int attempt = 0; attempt < 1; ++attempt) {
        // 송신
		//std::cout << "Attempt " << attempt + 1 << " to send data..." << std::endl;
		if (sendByteData(buffer, length) < 0) {
			continue;
        }
        // 수신 시도
   //     int recvLen = recvfrom(sock_, recvBuffer, sizeof(recvBuffer) - 1, 0, (sockaddr*)&fromAddr, &fromLen);
   //     if (recvLen > 0) {
   //         //ACK파싱 안했음 . eventcode읽어서 ack인거 확인 ..
			//std::cout << "ACK received from " << inet_ntoa(fromAddr.sin_addr) << ":" << ntohs(fromAddr.sin_port) << std::endl;
   //         return true;
   //     }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

	std::cout << "No ACK received after 10 attempts" << std::endl;
    return false;
}

void TCC::UdpMulticastSender::sendEmergencyDestroyCommand(std::string& commandId, std::string& missileId) {
    char * buffer = new char[100];

    int headerSize = serializeHeader(buffer, EventCode::emergencyDestroyCommand, sizeof(EmergencyDestroyCommandBody));
    int bodySize = serializeEmergencyDestroyCommandBody(buffer + headerSize, commandId, missileId);
    
	int totalSize = headerSize + bodySize;
    std::thread([this, buffer, totalSize, missileId]() {
        bool ackReceived = sendUntilReceiveAck(buffer, totalSize);
        setAckResult(missileId, ackReceived); // 결과 저장 및 알림
        //     if sendUntilReceiveAck(buffer, totalSize) {
        //         //성공
                 ////cv를 이용해서 EngagementManager에 알리기
        //     }
        //     else {
        //         //실패
                 //// cv를 이용해서 EngagementManager에 알리기
        //     }
        delete[] buffer;
        }).detach();

    return;
}

const int TCC::UdpMulticastSender::serializeHeader(char* buffer, unsigned int eventCode, int bodyLength) {
    memcpy(buffer, &eventCode, sizeof(unsigned int));
    memcpy(buffer + 4, &bodyLength, sizeof(int));
    return 8;
}

const int TCC::UdpMulticastSender::serializeLauncCommandBody(char* buffer, std::string& commandId, std::string& aircraftId, std::string& missileId, TCC::Position& impactPoint) {
    memcpy(buffer, commandId.c_str(), 20);
    memcpy(buffer + 20, aircraftId.c_str(), 8);
    memcpy(buffer + 28, missileId.c_str(), 8);
    memcpy(buffer + 36, &impactPoint, 24);
    return 60;
}

const int TCC::UdpMulticastSender::serializeEmergencyDestroyCommandBody(char* buffer, std::string& commandId, std::string& missileId) {
    memcpy(buffer, commandId.c_str(), 20);
    memcpy(buffer + 20, missileId.c_str(), 8);
    return 28;
} 

const int TCC::UdpMulticastSender::sendByteData(const char* data, int length) {
    if (sock_ == INVALID_SOCKET) {
        std::cout << "Socket is not initialized"<<std::endl;
        return -1;
    }

    int bytesSent = sendto(sock_, data, length, 0, (sockaddr*)&destAddr_, sizeof(destAddr_));
    if (bytesSent == SOCKET_ERROR) {
        std::cout << "sendto() failed: " << WSAGetLastError() << std::endl;
        return -1;
    }
    else {
		std::cout << "Sent " << bytesSent << " bytes to " << inet_ntoa(destAddr_.sin_addr) << ":" << ntohs(destAddr_.sin_port) << std::endl;
    }
    return bytesSent;
}
