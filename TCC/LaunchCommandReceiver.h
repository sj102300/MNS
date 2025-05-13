#pragma once

#include "TcpReceiver.h"
#include "share.h"

class LaunchCommandReceiver : public TCC::TcpReceiver {
private:
	typedef struct _launch_command_message {
		char launchCommandId_[20];	//20ÀÚ
		char aircraftId[8];		//8ÀÚ
	} LaunchCommandMSG;

public:
	LaunchCommandReceiver(std::string ip_address, int port);

	void start() override;

	void receive() override;

	bool parseMsg(LaunchCommandMSG& msg, const char* buffer, const int length);

	void work(LaunchCommandMSG& msg);

	void response() override;
};	