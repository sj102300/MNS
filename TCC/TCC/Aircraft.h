#pragma once

#include <string>
#include <cmath>
#include "share.h"

class Aircraft{
public:

	enum EngagementStatus {
		NotEngageable = 0,   // 교전 불가능 (IP가 범위 밖)
		Engageable = 1,      // 교전 가능 (IP가 범위 안)
		Engaging = 2,        // 교전 중 (추격 중인 미사일이 있음)  //여러 미사일이 하나의 항공기 격추 가능?
		Destroyed = 3        // 격추 완료 (미사일이 격추 성공함)
	};

	Aircraft(std::string id, TCC::Position pos_, bool isEnemy_);
	const bool isEnemy() const;
	void calcImpactPoint();
	bool isIpInEngageRange(unsigned int engagementStatus, TCC::Position& iplocation);
	void updatePosition(TCC::Position& newLocation);
	bool isEngagable();
	unsigned int updateStatus(unsigned int engagementStatus);

private:
	void calcDirVec(const TCC::Position& newPos);

	typedef struct _direction_vector {
		double dx_;
		double dy_;
		bool isZeroVector() {
			return dx_ == 0 && dy_ == 0;
		}
	} DirVector;

	std::string aircraftId_;
	TCC::Position pos_;
	DirVector dirVec_;
	bool isEnemy_;
	unsigned int status_;
	TCC::Position impactPoint_;
};
