#pragma once

#include "TcpSender.h"

class LaunchCommandSender : public TCC::TcpSender {
private:
    typedef struct _launch_command_packet {
        char fireCommandId[20]; // 발사명령 식별자
		char aircraftId[8]; // 항공기 식별자

        //패킷 초기화
		_launch_command_packet(const std::string& fireCmd, const std::string& aircraft) {
			memset(fireCommandId, 0, sizeof(fireCommandId));
			memset(aircraftId, 0, sizeof(aircraftId));

			// 문자열 복사 ( 안전하게 최대 길이 만틈)
			strncpy_s(fireCommandId, fireCmd.c_str(), sizeof(fireCommandId) - 1);
			strncpy_s(aircraftId, aircraft.c_str(), sizeof(aircraftId) - 1);
		}
    }LaunchCommandPacket;

public:
    LaunchCommandSender(std::string ip_address, int port);

    void start() override;

	void getResponse() override;
};