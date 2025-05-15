#pragma once

#include <string>
#include <cmath>
#include "share.h"

class Aircraft{
public:
	Aircraft(std::string id, TCC::Position pos_, bool isEnemy_);
	const bool isEnemy() const;
	void getImpactPoint();
	bool isIpInEngageRange(unsigned int engagementStatus, TCC::Position& iplocation);
	//bool isEngagable();

private:
	void calcDirVec(const TCC::Position& newPos);

	typedef struct _direction_vector {
		double dx_;
		double dy_;
		bool isZeroVector() {
			return dx_ == 0 && dy_ == 0;
		}
	} DirVector;

	enum class EngagementStatus : unsigned int{
		NotEngageable = 0,   // ���� �Ұ��� (IP�� ���� ��)
		Engageable = 1,      // ���� ���� (IP�� ���� ��)
		Engaging = 2,        // ���� �� (�߰� ���� �̻����� ����)
		Destroyed = 3        // ���� �Ϸ� (�̻����� ���� ������)
	};

	std::string aircraftId_;
	TCC::Position pos_;
	DirVector dirVec_;
	bool isEnemy_;
	EngagementStatus status_;
	TCC::Position impactPoint_;
};
