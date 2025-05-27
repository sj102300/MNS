#include "ShootDownCheck.h"

std::pair<double, double> ShootDownCheck::MissileReciever() {
	// 실제 미사일 수신 코드 있어야 함 (각 항공기에 맞는 미사일의 데이터를 수신해야 함)
	// 수신 데이터중 위/경도 좌표만 리턴

	std::pair<double, double> missileCoordinate = {0.1, 0.2};
	return missileCoordinate;
}

std::string ShootDownCheck::MissileReciever(std::string id) {

	std::string missileID = "MSS-0001";
	return missileID;
}