#include "ScenarioInit.h"

int main() {
    ScenarioInit scenarioRunner(
        "http://127.0.0.1:9015",  // 내 시나리오 수신 주소
        "http://127.0.0.1:8000",  // 요청할 SCN 서버 주소
        "MFR"                     // 내 클라이언트 ID
    );
    scenarioRunner.run();

    std::cin.get(); // 프로그램 종료 임시 방지
    return 0;
}
