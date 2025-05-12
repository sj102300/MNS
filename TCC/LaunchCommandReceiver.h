#pragma once

#include "TcpReceiver.h"
class LaunchCommandReceiver : public TCC::TcpReceiver {
private:
	typedef struct _launch_command_message {
		std::string launchCommandId;	//20ÀÚ
		std::string aircraftId;		//8ÀÚ
	} LaunchCommandMSG;

public:
	LaunchCommandReceiver(std::string ip_address, int port);

	void parseMsg(LaunchCommandMSG& msg, const char* buffer, int length);

	void receive() override;

	void start() override;

	void response() override;
};
