#include <iostream>
#include <vector>
#include <Windows.h>

// send_start_signal 함수 선언
void send_start_signal(const std::string& target_url, const std::string& client_id);

int main() {
    SetConsoleOutputCP(CP_UTF8);  // 콘솔 UTF-8 설정

    std::cout << u8"[OCC] 부체계에 시작신호 전송 시작...\n";

    // 부체계 정보: IP와 포트는 각자 다르게 설정
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

    std::cout << u8"[OCC] 전송 완료. 아무 키나 누르면 종료합니다.\n";
    std::cin.get();
    return 0;
}
