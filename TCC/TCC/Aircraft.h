#pragma once

#include <string>
#include <cmath>
#include "share.h"

class Aircraft{
public:

	enum EngagementStatus {
		NotEngageable = 0,   // ���� �Ұ��� (IP�� ���� ��)
		Engageable = 1,      // ���� ���� (IP�� ���� ��)
		Engaging = 2,        // ���� �� (�߰� ���� �̻����� ����)  //���� �̻����� �ϳ��� �װ��� ���� ����?
		Destroyed = 3        // ���� �Ϸ� (�̻����� ���� ������)
	};

	Aircraft(std::string id, TCC::Position pos_, bool isEnemy_);
	const bool isEnemy() const;
	bool calcImpactPoint(TCC::Position &batteryLoc);
	bool calcWDLImpactPoint(TCC::Position& curMissileLoc);
	bool hasBecomeEngageable(TCC::Position &batteryLoc, unsigned int& engagementStatus);
	void updatePosition(TCC::Position& newLocation);
	bool isEngagable();
	unsigned int updateStatus(unsigned int engagementStatus);
	void getImpactPoint(TCC::Position &impactPoint);
	~Aircraft();

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
