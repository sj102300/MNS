#include <iostream>
#include "EngagementManager.h"
#include "UdpReceiver.h"

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

	std::string missileId = missileManager_->findAvailableMissile();
	
	if (missileId.empty()) {
		return false;		//��밡���� �̻����� ����
	}

	//WDL��� �ҰŸ� �̹� �ִ� �̻������� �˻��ϱ�
	missileToAircraft_[missileId] = aircraftId;
	return true;
}

// MissileManager���� �̻����� ���°� ���� ������ �� �� �Լ��� ȣ��
// 
bool EngagementManager::isHitTarget(std::string& missileId) {
	//���� ���� ����
	//�ش� �̻����� �����ϴ� �װ��⸦ �ùٸ��� �����ߴ����� �Ǵ�.
	//���� �ùٸ��� �����ߴٸ� �װ����� status�� EngagementStatus::Destroyed
	//�ùٸ��� ���ߵ��� �ʾҴٸ� �װ����� status�� EngagementStatus::NotEngagable
	// 
	// �̻���-�װ��� ���� Ȯ��
	auto it = missileToAircraft_.find(missileId);
	if (it == missileToAircraft_.end()) {
		// ���ε� �װ��Ⱑ ����
		return false;
	}

	std::string& aircraftId = it->second;
	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if (!aircraft) {
		// �װ��� ��ü�� �������� ����
		missileToAircraft_.erase(it);
		return false;
	}

	// ������ ������ �װ��Ⱑ ���� �Ѵ� �װ��Ⱑ �´��� Ȯ���ϴ� ����
	
	//���� �Ѵ� �װ����� EngagementStatus�� Engagable or NotEngagable�� ����
	//�ٵ� �ΰ��� �̻����� �ϳ��� �Ѵٰ� �ϳ���	�����ߴٸ� Engagable? NotEngagable?
	//Todo : ���� �Ѵ� �װ��Ⱑ �ƴ� �װ��⸦ ���� �� ���  
	// �ùٸ��� �����ߴٰ� �Ǵ�
	std::cout << aircraftId << " �̻��� ���� ���� " << "\n";

	// ���� ����
	missileToAircraft_.erase(it);

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

// OCC���� ������� ��Ŷ�� �������� �� ������� ����� ��Ƽĳ��Ʈ�� ����
// ������� �Ϸ�� Ack(�̻��Ͽ��� ������� ���¸� ��������)�޾��� �� �Ǵ�
bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	//��� ���� ��� �۽�
	Missile* missile = missileManager_->selectMissile(missileId);

	sender_->sendEmergencyDestroy(commandId, missileId);

	missileToAircraft_.erase(missileId);

	// OCC -> TCC ������� ����� ������
	// TCC -> OCC ������� ����� �޾Ҵٴ� Ack ������
	// 
	// OCC->TCC ������� ok, TCC -> MSS �̰� ���޾�, OCC�� GUI ���¸� ���� ACk�� �Ƚ��
	// TCC -> �ùķ��̼� �� ������� ����� �Ѹ���
	// 
	// TCC receiver �̻����� �����͸� ��� ���� -> status == �������
	// �� ������İ� ��� �̻����� ����������� Ȯ�� -> Ack
	//sender���� �̻��� ���¸� �����ͼ� status ������� -> true -> ���� ó�� cv
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