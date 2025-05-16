
#include "AircraftManager.h"

AircraftManager::AircraftManager(IAircraftReceiver* recv, IAircraftSender* sender) : receiver_(recv), sender_(sender),  {

}

void AircraftManager::startAircraftSimulation() {
	receiver_->recvAircraftData();
	workThread_ = std::thread(&AircraftManager::judgeEngagable, this);
	sender_->sendAircraftData();
}

bool AircraftManager::isExistAircraft(std::string& aircraftId) {
	return aircrafts_.find(aircraftId) != aircrafts_.end();
}

void AircraftManager::addAircraft(IAircraftReceiver::NewAircraft &newAircraft) {
	aircrafts_[newAircraft.aircraftId_] = new Aircraft(newAircraft.aircraftId_, newAircraft.location_, newAircraft.isEnemy_);
}

void AircraftManager::judgeEngagable() {

	IAircraftReceiver::NewAircraft newAircraft;
	IAircraftSender::NewAircraftWithIP newAircraftWithIp;

	while (true) {
		if (receiver_->popRecvQueue(newAircraft)) {

			if (!isExistAircraft(newAircraft.aircraftId_)) {
				addAircraft(newAircraft);
			}

			Aircraft* targetAircraft = aircrafts_[newAircraft.aircraftId_];
			targetAircraft->updatePosition(newAircraft.location_);
			
			newAircraftWithIp.aircraftId_ = newAircraft.aircraftId_;
			newAircraftWithIp.location_ = newAircraft.location_;
			newAircraftWithIp.isEnemy_ = newAircraft.isEnemy_;

			if (!targetAircraft->isEnemy()) {	//�Ʊ� �װ���
				sender_->pushSendQueue(newAircraftWithIp);
				return;
			}
			
			if (targetAircraft->isIpInEngageRange(newAircraftWithIp.engagementStatus_, newAircraftWithIp.impactPoint_)) {	//���� �װ��� �� ���� ���� ���� �ƴ�
				sender_->pushSendQueue(newAircraftWithIp);
				return;
			}

			sender_->pushSendQueue(newAircraftWithIp);	//���� �װ��� �� ���� ������ �װ���
			//engagementManager_->addEngagableAircraft(newAircraftWithIp.aircraftId_);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

}

void AircraftManager::updateAircraftStatus() {

}