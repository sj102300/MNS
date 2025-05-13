#pragma once
class ShootDownChecker {
public:
	virtual ~ShootDownChecker() {}
	virtual void ShootDown() = 0;
	virtual void SendSuccessInfo() = 0;
	virtual void MissileReciever() = 0;
};