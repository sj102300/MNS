#include "ScenarioInit.h"

int main() {
    ScenarioInit scenarioRunner(
        "http://127.0.0.1:9015",  // �� �ó����� ���� �ּ�
        "http://127.0.0.1:8000",  // ��û�� SCN ���� �ּ�
        "MFR"                     // �� Ŭ���̾�Ʈ ID
    );
    scenarioRunner.run();

    std::cin.get(); // ���α׷� ���� �ӽ� ����
    return 0;
}
