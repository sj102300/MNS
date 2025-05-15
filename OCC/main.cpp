#include <iostream>
#include <vector>
#include <Windows.h>

// send_start_signal �Լ� ����
void send_start_signal(const std::string& target_url, const std::string& client_id);

int main() {
    SetConsoleOutputCP(CP_UTF8);  // �ܼ� UTF-8 ����

    std::cout << u8"[OCC] ��ü�迡 ���۽�ȣ ���� ����...\n";

    // ��ü�� ����: IP�� ��Ʈ�� ���� �ٸ��� ����
    std::vector<std::pair<std::string, std::string>> clients = {
        //{ "http://localhost:9011", "TCC" },
        //{ "http://localhost:9012", "MSS" },
        //{ "http://localhost:9013", "ATS" },
        //{ "http://localhost:9014", "LCH" },
        { "http://localhost:9015", "MFR" }
    };

    for (const auto& [url, id] : clients) {
        send_start_signal(url, id);
    }

    std::cout << u8"[OCC] ���� �Ϸ�. �ƹ� Ű�� ������ �����մϴ�.\n";
    std::cin.get();
    return 0;
}
