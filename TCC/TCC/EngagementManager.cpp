

#include "EngagementManager.h"

void EngagementManager::startEngagementSimulation() {
	//commandreceiver_����
	//successreceiver_����
}

void EngagementManager::mappingMissileToAircraft(std::string& aircraftId) {
	//missilemanager�κ��� selected�� missile�� �޾Ƽ�
	//missileToAircraft_  map�� ���� key�� missileId, value�� aircraftId;
}

bool EngagementManager::isHitTarget() {
	//���� ���� ����
	//�ش� �̻����� �����ϴ� �װ��⸦ �ùٸ��� �����ߴ����� �Ǵ�.
	//���� �ùٸ��� �����ߴٸ� �װ����� status�� EngagementStatus::Destroyed
	//�ùٸ��� ���ߵ��� �ʾҴٸ� �װ����� status�� EngagementStatus::NotEngagable
}

bool EngagementManager::changeMode() {
	//��� ��ȯ �ϱ�..
}

void EngagementManager::launchMissile(std::string& aircraftId) {
	//selectMissile()�ؼ� ����� �̻��� ����
	// mappingMissileToAircraft() �ϱ�
	//missileId�� missileToAircraft �ʿ��� ������ �װ��⸦ ã�Ƽ�
	//�߻� ��� �۽�
}

void EngagementManager::emergencyDestroy(std::string & missileId) {
	//��������� �̻��Ͽ� ���ؼ�
	//��� ���� ��� �۽�
}

void EngagementManager::addEngagableAircraft(std::string& AircraftId) {
	//ť�� Aircraft push
	//�̰� ť�� ���� �����ؼ� �ߺ� �Ұ����� ť�� ���� ����Ұ�
}