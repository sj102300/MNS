

#include "EngagementManager.h"

void EngagementManager::startEngagementSimulation() {
	//commandreceiver_시작
	//successreceiver_시작
}

void EngagementManager::mappingMissileToAircraft(std::string& aircraftId) {
	//missilemanager로부터 selected된 missile을 받아서
	//missileToAircraft_  map에 저장 key가 missileId, value가 aircraftId;
}

bool EngagementManager::isHitTarget() {
	//격추 성공 수신
	//해당 미사일이 격추하던 항공기를 올바르게 격추했는지를 판단.
	//만약 올바르게 격추했다면 항공기의 status를 EngagementStatus::Destroyed
	//올바르게 격추되지 않았다면 항공기의 status를 EngagementStatus::NotEngagable
}

bool EngagementManager::changeMode() {
	//모드 전환 하기..
}

void EngagementManager::launchMissile(std::string& aircraftId) {
	//selectMissile()해서 사용할 미사일 선택
	// mappingMissileToAircraft() 하기
	//missileId로 missileToAircraft 맵에서 격추할 항공기를 찾아서
	//발사 명령 송신
}

void EngagementManager::emergencyDestroy(std::string & missileId) {
	//비상폭파할 미사일에 대해서
	//비상 폭파 명령 송신
}

void EngagementManager::addEngagableAircraft(std::string& AircraftId) {
	//큐에 Aircraft push
	//이거 큐는 직접 구현해서 중복 불가능한 큐를 만들어서 사용할것
}