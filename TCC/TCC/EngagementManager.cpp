
#include "EngagementManager.h"

bool EngagementManager::init(TCC::UdpSender* sender) {
	if (sender == nullptr)
		return false;
	sender_ = sender;
	return true;
}

void EngagementManager::start() {
	workThread_ = std::thread(&EngagementManager::work, this);
}

void EngagementManager::work() {

	while (true) {
		std::unique_lock<std::mutex> lock(mtx_);

		// mode�� 0�̰� �۾��� ���� ���� ���
		cv_.wait(lock, [this] {
			return mode_ == Mode::Auto && !engagableAircrafts_.empty();
		});

		// queue���� �۾� �ϳ� ����
		std::string aircraftId;
		engagableAircrafts_.popQueue(aircraftId);
		lock.unlock();

		// ���� �۾� ó��
		launchMissile(aircraftId);
	}
}

bool EngagementManager::mappingMissileToAircraft(std::string& aircraftId) {

	//std::string missileId = missileManager->selectMissile();
	
	//if (missileId.empty()) {
	//	return false;		//��밡���� �̻����� ����
	//}

	//WDL��� �ҰŸ� �̹� �ִ� �̻������� �˻��ϱ�
	//missileToAircraft_[missileId] = aircraftId;
	return true;
}

bool EngagementManager::isHitTarget() {
	//���� ���� ����
	//�ش� �̻����� �����ϴ� �װ��⸦ �ùٸ��� �����ߴ����� �Ǵ�.
	//���� �ùٸ��� �����ߴٸ� �װ����� status�� EngagementStatus::Destroyed
	//�ùٸ��� ���ߵ��� �ʾҴٸ� �װ����� status�� EngagementStatus::NotEngagable
	return true;
}

unsigned int EngagementManager::changeMode(unsigned int mode) {
	mode_ = mode;
	if (mode_ == Mode::Auto) {
		cv_.notify_one();
	}
	else {
		engagableAircrafts_.clear();
	}
	return mode_;
}

bool EngagementManager::manualFire(std::string commandId, std::string targetAircraftId) {
	std::cout << "manaulFire() : " << commandId << std::endl;
	return launchMissile(targetAircraftId);
}

bool EngagementManager::launchMissile(std::string& aircraftId) {

	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if (!aircraft->isEngagable()) {
		return false;		//���� �Ұ��� �� �װ�����.
	}

	mappingMissileToAircraft(aircraftId);
	unsigned int engagementStatus = aircraft->updateStatus(Aircraft::EngagementStatus::Engaging);
	//sender_->sendLaunchCommand();
	return true;
}

bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	//��������� �̻��Ͽ� ���ؼ�
	//��� ���� ��� �۽�
	
	//Missile* missile = missileManager_->getMissile(missileId);

	//sender_->sendEmergencyDestroy(missileId);

	missileToAircraft_.erase(missileId);

	//return false?true/
	return true;
}

void EngagementManager::addEngagableAircraft(std::string& aircraftId) {
	if (mode_ == Mode::Manual)
		return;

	if (engagableAircrafts_.pushQueue(aircraftId))
		cv_.notify_one();
}

bool EngagementManager::EngagableAircraftQueue::pushQueue(std::string& aircraftId) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (set.find(aircraftId) != set.end())
		return false;
	set.insert(aircraftId);
	queue.push(aircraftId);
	return true;
}

bool EngagementManager::EngagableAircraftQueue::popQueue(std::string& aircraftId) {
	std::lock_guard<std::mutex>lock(mtx_);
	if (queue.empty())
		return false;
	aircraftId = queue.front();
	queue.pop();
	set.erase(aircraftId);
	return true;
}

bool EngagementManager::EngagableAircraftQueue::empty() {
	std::lock_guard<std::mutex>lock(mtx_);
	return queue.empty();
}

void EngagementManager::EngagableAircraftQueue::clear() {
	std::lock_guard<std::mutex>lock(mtx_);
	set.clear();
	while (!queue.empty()) {
		queue.pop();
	}
}