#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "SCNclient.h"

#include <windows.h>
#include <iostream>

int main() {
	SetConsoleOutputCP(CP_UTF8);  // �ܼ��� UTF-8 ������� ����
	
	request_scenario();

	std::cout << u8"\n���α׷� ����... �ƹ� Ű�� ��������.\n";
	std::cin.get();
	return 0;
}
