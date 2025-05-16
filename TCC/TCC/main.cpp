
#include "ScenarioManager.h"

int main() {

    ScenarioManager scenarioManager;

    scenarioManager.startScenario();


    while (true) {
        //cv.wait(시나리오 시작 시그널 받기대기)
        //scenarioManager.startScenario();

        //cv.wait(시나리오 종료 시그널 받기 대기)
        //scenarioManager.quitScenario();
    }

    while (1);

    return 0;
}
