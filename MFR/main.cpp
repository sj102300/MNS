#include "ScenarioInit.h"
#include <condition_variable>
#include <mutex>
#include <iostream>

using namespace sm;

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

int main() {
    ScenarioInit scenarioRunner(
        "http://127.0.0.1:9015",  // 내 시나리오 수신 주소
        "http://127.0.0.1:8000",  // 요청할 SCN 서버 주소
        "MFR"                     // 내 클라이언트 ID
    );
    
    scenarioRunner.setOnReadyCallback([&]() {  // 콜백 등록
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
        cv.notify_one();
    });

    scenarioRunner.run();

    // 메인 스레드에서 대기
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });

    ScenarioInfo info = scenarioRunner.getScenarioInfo();
    Coordinate battery = scenarioRunner.getBatteryLocation();
    std::vector<AircraftInfo> aircrafts = scenarioRunner.getAircraftList();

    //////////////////////////////////  출력 확인  ///////////////////////////////////
    std::cout << "\n" << u8"===============================================" << "\n";
    std::cout << u8"시나리오 ID는: " << info.scenario_id << std::endl;
    std::cout << u8"[포대 위치] 위도: " << battery.latitude
        << u8", 경도: " << battery.longitude
        << u8", 고도: " << battery.altitude << std::endl;
    for (const auto& ac : aircrafts) {
        std::cout << u8"[항공기] ID: " << ac.aircraft_id << u8", 피아: " << ac.friend_or_foe << std::endl;
    }
    //////////////////////////////////////////////////////////////////////////////////




    std::cin.get(); // 프로그램 종료 임시 방지
    return 0;
}
