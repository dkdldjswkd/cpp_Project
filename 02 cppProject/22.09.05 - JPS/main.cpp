#include "stdafx.h"
#include "WndProc.h"

int main() {
	setlocale(LC_ALL, "korean");
	system(" mode  con lines=20   cols=50 ");

	Init_window();

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}