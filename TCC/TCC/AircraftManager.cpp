
#include "AircraftManager.h"

AircraftManager::AircraftManager(IAircraftReceiver* recv, IAircraftSender* sender) : receiver_(recv), sender_(sender) {

}

void AircraftManager::updateAircraftPosition() {
	receiver_->getAircraftData();
	workThread_ = std::thread(&AircraftManager::judgeEngagable, this);
	sender_->sendAircraftData();
}

bool AircraftManager::isExistAircraft(std::string& aircraftId) {
	return aircrafts_.find(aircraftId) != aircrafts_.end();
}

void AircraftManager::addAircraft(std::string& aircraftId, IAircraftReceiver::AircraftMSG &msg) {
	aircrafts_[aircraftId] = new Aircraft(aircraftId, msg.location, msg.isEnemy);
}

void AircraftManager::judgeEngagable() {

	IAircraftReceiver::AircraftMSG recvmsg;

	while (true) {
		if (receiver_->popRecvQueue(recvmsg)) {

			std::string aircraftId = std::string(recvmsg.aircraftId, 8);

			if (!isExistAircraft(aircraftId)) {
				addAircraft(aircraftId, recvmsg);
			}

			Aircraft* targetAircraft = aircrafts_[aircraftId];
			IAircraftSender::AircraftMSG sendmsg;
			memcpy(&sendmsg, reinterpret_cast<const char*>(&recvmsg), 32);

			if (!targetAircraft->isEnemy()) {	//�Ʊ� �װ���
				sendmsg.isEnemy_ = 0;
				sender_->pushSendQueue(sendmsg);
				return;
			}

			sendmsg.isEnemy_ = 1;
			if (targetAircraft->isIpInEngageRange(sendmsg.engagementStatus_, sendmsg.impactPoint_)) {	//���� �װ��� �� ���� ���� ���� �ƴ�
				sender_->pushSendQueue(sendmsg);
				return;
			}

			sender_->pushSendQueue(sendmsg);	//���� �װ��� �� ���� ������ �װ���

			//EngagementManager�� ť�� �ֱ�
			


		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

}

void AircraftManager::updateAircraftStatus() {

}