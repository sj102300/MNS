#include "ATS.h"
#include "AircraftManager.h"
#include "Missile.h"  // globalMissiles, missileMtx

void ATS::setAircraftList(const std::vector<ats::AircraftInfo>& list) {
    aircrafts_ = list;
}

void ATS::launchAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    // �ó����� ���� �� ���� �̻��� ���� �ʱ�ȭ
    {
        std::lock_guard<std::mutex> mlock(missileMtx);
        globalMissiles.clear();
    }

    shootDownThread_.start();  // ���� �Ǵ� ������ ����

    // �װ��� ��ġ ������Ʈ ������ ����
    runAircraftManagerThread(std::move(aircrafts_), &shootDownThread_);
}

void ATS::terminateAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    // �� �̻� �װ��� ������� �����Ƿ� worker ���� ����
    shootDownThread_.stop();  // �������� ���� �Ǵ� ������ ����
}
