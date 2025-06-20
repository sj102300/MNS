#pragma once
#include "share.h"
#include <string>

//�̻��Ϸα�â����,, ���������� �ٲ��,, �޽���? ����

class Missile {
public:
	Missile(std::string id, TCC::Position pos_, int status, TCC::Position impact);

	enum MissileStatus : unsigned int {
		Wait = 0,   // ���(�߻� ��)
		Fly = 1,      // ����(ǥ���� ���� ���� ��)
		Splashed = 2,        // ����(�̻����� ǥ�� ����)
		EmergencyDestroyed = 3,        // �������(����ڰ� ������� ��� ����)
		SelfDestroyed = 4,        // ���� (IP������ �� �ڵ� ����)
		FollowUp = 5,				//���� ��
		LaunchRequest = 6,			//�߻� ��û ��
		Turning = 7, //��ȸ��, WDL
	};

	bool checkId(std::string id);
	bool isAvailable(unsigned int status, std::string& missileId);
	void updateStatus(MissileStatus);
	unsigned int getMissileStatus();
	bool getCurLocation(TCC::Position& loc);
	void updatePosition(TCC::Position& newloc);
	~Missile();

private:
	/*typedef struct _direction_vector {
		double dx_;
		double dy_;
		bool isZeroVector() {
			return dx_ == 0 && dy_ == 0;
		}
	} DirVector;*/

	std::string missileId_;
	TCC::Position position_;
	MissileStatus status_;
	TCC::Position impactPoint_;
};