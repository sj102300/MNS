#pragma once
#include "Packet.h"
#include <thread>
#include <mutex>

#include "DestroyedAircrafts.h"

class MissileController;
class UdpMulticast;

class Missile : public std::enable_shared_from_this <Missile> {
public:
    std::string MissileId;
    unsigned int MissileState;
    Location MissileLoc;
    std::string TargetAircraftId;  // [�߰�] Ÿ�� �װ��� ID
    std::shared_ptr<MissileController> getController();  // �� �� �� �߰�
	std::mutex mtx;  // ������ ������ ���� ���ؽ�

    // �̻��� �������ڸ��� -> �ٷ� ������ ������ �����Ѵ�!!
    Missile();

    void setMissileId(const std::string& id);

    void setState(unsigned int state);

    void setLoc(Location& loc);

    void setTargetLocation(const Location& loc);

    // �ʱ�ȭ�ϴ� ���
    void init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c);

    // [�߰�] Ÿ�� �װ��� ID ����
    void setTargetAircraftId(const std::string& aircraftId);
    const std::string& getTargetAircraftId() const;

    // �����ϴ� ���
    void start(float speed);

    void stop();
private:
    std::shared_ptr<MissileController> controller;
    std::shared_ptr<UdpMulticast> sender;
};