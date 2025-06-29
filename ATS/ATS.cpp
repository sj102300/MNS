#include "ATS.h"
#include "AircraftManager.h"
#include "Missile.h"  // globalMissiles, missileMtx

void ATS::setAircraftList(const std::vector<ats::AircraftInfo>& list) {
    aircrafts_ = list;
}

void ATS::launchAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    // 시나리오 시작 전 이전 미사일 정보 초기화
    {
        std::lock_guard<std::mutex> mlock(missileMtx);
        globalMissiles.clear();
    }

    shootDownThread_.start();  // 격추 판단 스레드 시작

    // 항공기 위치 업데이트 스레드 실행
    runAircraftManagerThread(std::move(aircrafts_), &shootDownThread_);
}

void ATS::terminateAll() {
    std::lock_guard<std::mutex> lock(mtx_);

    // 더 이상 항공기 스레드는 없으므로 worker 관련 삭제
    shootDownThread_.stop();  // 마지막에 격추 판단 스레드 종료
}
