#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UdpReceiver.h"
#include "Packet.h"
#include "MissileController.h" // [추가]

UdpReceiver::UdpReceiver(DestroyedAircraftsTracker * tracker)
    : sock_(INVALID_SOCKET), running_(false), port_(0), tracker_(tracker) {
}

bool UdpReceiver::init(const std::string& multicast_address, int port) {
    port_ = port;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        return false;
    }

    // 재사용 가능하도록 설정
    int reuse = 1;
    if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt() failed\n";
        return false;
    }

    // 로컬 주소 설정
    memset(&localAddr_, 0, sizeof(localAddr_));
    localAddr_.sin_family = AF_INET;
    localAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr_.sin_port = htons(port_);

    if (bind(sock_, (struct sockaddr*)&localAddr_, sizeof(localAddr_)) < 0) {
        std::cerr << "bind() failed\n";
        return false;
    }

    // 멀티캐스트 그룹 가입
    mreq_.imr_multiaddr.s_addr = inet_addr(multicast_address.c_str());
    mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_)) < 0) {
        std::cerr << "setsockopt(IP_ADD_MEMBERSHIP) failed\n";
        return false;
    }

    return true;
}

void UdpReceiver::start() {
    running_ = true;
    recvThread_ = std::thread(&UdpReceiver::run, this);
}

void UdpReceiver::run() {
    char buffer[2048];  // 충분히 넉넉한 크기로
    while (running_) {
        sockaddr_in senderAddr;
        int senderLen = sizeof(senderAddr);
        int bytesReceived = recvfrom(sock_, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderLen);

        if (bytesReceived > 8) {
            // 1. eventCode와 bodyLength 파싱
            uint32_t eventCode;
            uint32_t bodyLength;

            memcpy(&eventCode, buffer, sizeof(unsigned int));
            memcpy(&bodyLength, buffer + 4, sizeof(unsigned int));

            if (bytesReceived < 8 + static_cast<int>(bodyLength)) {
                std::cerr << u8"잘못된 패킷 길이\n";
                continue;
            }

            //const char* bodyPtr = buffer + 8;
            const char* bodyPtr = buffer;

            switch (eventCode) {
            case 2002: { // 미사일 발사 명령 처리
                if (bodyLength < sizeof(OrderPacket)-8) {
                    std::cerr << u8"미사일 패킷 길이 부족\n";
                    //std::cout << u8"packet size : " << sizeof(OrderPacket) << "\n";
                    break;
                }

                OrderPacket orderpkg;
                memcpy(&orderpkg, bodyPtr, sizeof(orderpkg));
                std::string missileId(orderpkg.MissileId, strnlen(orderpkg.MissileId, 8));
                std::string aircraftId(orderpkg.AtsId, strnlen(orderpkg.AtsId, 8));
                Location impactPoint = orderpkg.ImpactPoint;

                std::cout << u8"[명령 수신] 미사일: " << missileId
                    << u8", 타겟 항공기: " << aircraftId << "\n";

                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    //std::cout << u8" 미사일 업데이트 하는중 \n";
                    auto missile = it->second;
                    missile->setMissileId(missileId);
                    missile->setState(1);  // 예: 3 = 발사 후 비행 상태
                    missile->setTargetLocation(impactPoint);
                
                    // 타겟 항공기 ID 설정
                    missile->setTargetAircraftId(aircraftId);

                    // 컨트롤러에도 항공기 ID 전달
                    auto controller = missile->getController();
                    if (controller) {
                        controller->setTargetAircraftId(aircraftId);

                        // aircraft map도 연결
                        controller->setAircraftMap(&Aircraft_map_);
                    }
                }
                //std::cout << u8"[업데이트됨] 미사일: " << missileId
                //    << u8" → 목표: (" << impactPoint.altitude << ", " << impactPoint.latitude << ")\n";
              
                break;
            }

            case 2003: { // 격추 여부 
                if (bodyLength < sizeof(InterceptResultPacket) - 8) {
                    std::cerr << u8"미사일 InterceptResultPacket 패킷 길이 부족\n";
                    //std::cout << u8"packet size : " << sizeof(InterceptResultPacket) << "\n";
                    break;
                }

                InterceptResultPacket Resultpkg;
                memcpy(&Resultpkg, bodyPtr, sizeof(Resultpkg));

                std::string missileId(Resultpkg.MissileId, strnlen(Resultpkg.MissileId, 8));

                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    //std::cout << u8" 미사일 업데이트 하는중 \n";
                    auto missile = it->second;
                    missile->setMissileId(missileId);
                    missile->setState(2);  // 2번은 격추 상태!!
                }
				std::string aircraftId(Resultpkg.AirCraftId, 8);
				tracker_->findNewDestroyedAircraft(aircraftId); // 격추된 항공기 ID를 기록

                std::cout << u8"[업데이트됨] 미사일: " << missileId << u8"격추 상태로 변환\n";

                break;
            }
            case 2004: { // 비상 폭파 명령 
                if (bodyLength < sizeof(EDPacket) - 8) {
                    std::cerr << u8"미사일 ED 패킷 길이 부족\n";
                    std::cout << u8"packet size : " << sizeof(EDPacket) << "\n";
                    break;
                }

                EDPacket EDpkg;
                memcpy(&EDpkg, bodyPtr, sizeof(EDpkg));

                std::string missileId(EDpkg.MissileId, strnlen(EDpkg.MissileId, 8));
                //std::cout << u8"수신한 미사일 ID: [" << missileId << "]\n";

                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    //std::cout << u8" 미사일 업데이트 하는중 \n";
                    auto missile = it->second;
                    missile->setMissileId(missileId);
                    missile->setState(3);  // 3번은 비상폭파 상태임
                }
                std::cout << u8"[업데이트됨] 미사일: " << missileId << u8"비상폭파 상태로 변환\n";

                break;
            }
            // ATS를 주기적으로 받기 위함임
            case 1001: {
                if (bodyLength < sizeof(AirCraftPacket) - 8) {
                    std::cerr << u8"항공기 패킷 길이 부족\n";
                    break;
                }
                AirCraftPacket acPacket;
                memcpy(&acPacket, bodyPtr, sizeof(acPacket));

                std::string aircraftId(acPacket.AtsId, strnlen(acPacket.AtsId, 8));
                Location loc = acPacket.AtsLoc;
                //Velocity vel = acPacket.AtsVelocity;

                auto it = Aircraft_map_.find(aircraftId);
                if (it != Aircraft_map_.end()) {
                    // 기존 객체가 있으면 업데이트
                    auto aircraft = it->second;
                    aircraft->update(loc);
                }
                else {
                    // 없으면 새 객체 만들어 추가
                    auto newAircraft = std::make_shared<Aircraft>(aircraftId, loc);
                    Aircraft_map_[aircraftId] = newAircraft;
                }

               // std::cout << u8"[항공기 업데이트] ID: " << aircraftId
               //     << u8" → 위치: (" << loc.latitude << ", " << loc.longitude << ", " << loc.altitude << ")\n";
            
                break;
            }
            case 301: { // IMPACT_POINT 재설정
                if (bodyLength < sizeof(WDLPacket) - 8) {
                    std::cerr << u8"WDL 패킷 길이 부족\n";
                    std::cout << u8"packet size : " << sizeof(WDLPacket) << "\n";
                    break;
                }

                WDLPacket wdlPacket;
                memcpy(&wdlPacket, bodyPtr, sizeof(wdlPacket));

                std::string missileId(wdlPacket.MissileId, strnlen(wdlPacket.MissileId, 8));
                Location newImpactPoint = wdlPacket.ImpactPoint;
                std::string aircraftId(wdlPacket.AtsId, strnlen(wdlPacket.AtsId, 8));
                auto it = missile_map_.find(missileId);
                if (it != missile_map_.end()) {
                    std::cout << u8"미사일 [" << missileId << u8"]의 목표 위치를 갱신합니다.\n";
                    auto missile = it->second;
                    missile->setState(7);  // 3번은 비상폭파 상태임
					std::thread([missile]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                        missile->setState(1);  // 1번은 발사 후 비행 상태
                    }).detach();
                    missile->setTargetLocation(newImpactPoint);
                   
                    missile->setTargetAircraftId(aircraftId);
                    /*방금 추가한 컨트롤러 코드*/
                    auto controller = missile->getController();
                    if (controller) {
                        controller->setTargetAircraftId(aircraftId);

                        // aircraft map도 연결
                        controller->setAircraftMap(&Aircraft_map_);
                    }
                    //std::cout << u8"[업데이트됨] 미사일: " << missileId
                    //    << u8" → 새로운 목표: (" << newImpactPoint.latitude << ", "
                    //    << newImpactPoint.longitude << ", "
                    //    << newImpactPoint.altitude << ")\n";
                }
                else {
                    std::cerr << u8"해당 미사일 ID를 찾을 수 없습니다: " << missileId << "\n";
                }

                break;
            }
            default:
                //std::cerr << u8"알 수 없는 이벤트 코드: " << eventCode << "\n";
                break;
            }
        }
    }
}

void UdpReceiver::close() {
    running_ = false;
    if (recvThread_.joinable()) {
        recvThread_.join();
    }

    // 멀티캐스트 그룹 탈퇴
    setsockopt(sock_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_));
    closesocket(sock_);
    WSACleanup();
}

void UdpReceiver::setMissileMap(const std::unordered_map<std::string, std::shared_ptr<Missile>>& map) {
    missile_map_ = map;
}

const std::unordered_map<std::string, std::shared_ptr<Aircraft>>* UdpReceiver::getAircraftMapPtr() const {
    return &Aircraft_map_;
}
