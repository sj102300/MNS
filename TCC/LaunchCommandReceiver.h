#pragma once

#include "TcpReceiver.h"

namespace LaunchCommand {
	typedef struct _launch_command_message {
		std::string launchCommandId;	//20ÀÚ
		std::string aircraftId;		//8ÀÚ
	} LaunchCommandMSG;

	class LaunchCommandReceiver : public TCC::TcpReceiver<LaunchCommandMSG> {
	public:
		LaunchCommandReceiver(std::string ip_address, int port) : TCC::TcpReceiver<LaunchCommandMSG>(ip_address, port) {}
		
		void parseMsg(LaunchCommandMSG &msg, const char* buffer, int length) override;

		void receive() override;

		void start() override;

		void response() override;

	};
}
