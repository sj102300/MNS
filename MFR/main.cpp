#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "StartSignalReceiver.h"
#include "MFR.h"  // request_scenario()

#include <windows.h>
#include <iostream>

// ���� ���
constexpr char ADDRESS[] = "http://localhost:9015";
constexpr char CLIENT_ID[] = "MFR";

int main() {
	SetConsoleOutputCP(CP_UTF8);  // �ܼ��� UTF-8 ������� ����
	

    setup_start_signal_listener(
        ADDRESS,    // MFR�� ��Ʈ
        CLIENT_ID,  // Ŭ���̾�Ʈ �ĺ���
        []() {
            request_scenario();  // ���� ��ȣ ���� �� ������ ����
        }
    );

    std::cout << u8"[" << CLIENT_ID << u8"] OCC���� ���۽�ȣ�� ��ٸ��ϴ�.\n";
	std::cin.get();
	return 0;
}
