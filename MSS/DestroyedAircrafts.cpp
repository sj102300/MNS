#include "DestroyedAircrafts.h"

void DestroyedAircraftsTracker::findNewDestroyedAircraft(std::string& aircraftId) {
    std::lock_guard<std::mutex> lock(destroyedAircraftsMutex);
	std::cout << u8"[DestroyedAircraftsTracker] �װ��� ID: " << aircraftId << "�� �ı��Ǿ����ϴ�.\n";
    destroyedAircrafts.insert(aircraftId);  // ���� Ÿ�� �װ��� ID�� �ı��� ��Ͽ� �߰�
}

bool DestroyedAircraftsTracker::isDestroyedAircraft(std::string & aircraftId) {
    std::lock_guard<std::mutex> lock(destroyedAircraftsMutex);
    if (destroyedAircrafts.find(aircraftId) != destroyedAircrafts.end()) {
		//std::cout << "[DestroyedAircraftsTracker] �װ��� ID: " << aircraftId << "�� �ı��� �װ����Դϴ�.\n";
        return true;
    }
    return false;
}

void DestroyedAircraftsTracker::clearDestroyedAircrafts() {
    std::lock_guard<std::mutex> lock(destroyedAircraftsMutex);
	destroyedAircrafts.clear();  // �ı��� �װ��� ��� �ʱ�ȭ
}