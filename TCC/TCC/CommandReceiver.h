#pragma once

#include "TcpReceiver.h"
#include "share.h"

class ICommandReceiver {
protected:
	typedef struct _mode_change_command_message {
		const unsigned int commandCode_ = 300;
		unsigned int mode;
	} ModeChangeCommandMSG;
	
	typedef struct _manual_fire_command_message {
		const unsigned int commandCode_ = 301;
		char manualFireId_[20];
		char aircraftId_[8];
	} ManualFireCommandMSG;
	
	typedef struct _emergency_destroy_command_message {
		const unsigned int commandCode_ = 302;
		char emergencyDestroyId_[20];
		char missileId_[8];
	} EmergencyDestroyCommandMSG;

	virtual void recvModeChangeCommand() = 0;
	virtual void recvManualFireCommand() = 0;
	virtual void recvEmergencyDestroyCommand() = 0;
};

class CommandReceiver : public TCC::TcpReceiver, ICommandReceiver {
public:
	CommandReceiver(std::string ip_address, int port);

	void receiveMessage();
	void recvModeChangeCommand() override;
	void recvManualFireCommand() override;
	void recvEmergencyDestroyCommand() override;

private:
	std::thread recvThread_;
	//EngagementManager* engagementManager_;
};