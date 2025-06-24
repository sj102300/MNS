#include "DestroyedAircrafts.h"

void DestroyedAircraftsTracker::findNewDestroyedAircraft(std::string& aircraftId) {
    std::lock_guard<std::mutex> lock(destroyedAircraftsMutex);
	std::cout << u8"[DestroyedAircraftsTracker] 항공기 ID: " << aircraftId << "가 파괴되었습니다.\n";
    destroyedAircrafts.insert(aircraftId);  // 현재 타겟 항공기 ID를 파괴된 목록에 추가
}

bool DestroyedAircraftsTracker::isDestroyedAircraft(std::string & aircraftId) {
    std::lock_guard<std::mutex> lock(destroyedAircraftsMutex);
    if (destroyedAircrafts.find(aircraftId) != destroyedAircrafts.end()) {
		//std::cout << "[DestroyedAircraftsTracker] 항공기 ID: " << aircraftId << "는 파괴된 항공기입니다.\n";
        return true;
    }
    return false;
}

void DestroyedAircraftsTracker::clearDestroyedAircrafts() {
    std::lock_guard<std::mutex> lock(destroyedAircraftsMutex);
	destroyedAircrafts.clear();  // 파괴된 항공기 목록 초기화
}