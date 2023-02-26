#include <iostream>
#include <Windows.h>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
using namespace std;

int main() {
	time_t t;
	for (;;) {
		//Sleep(1000);
		time(&t);
		cout << t << endl;
	}
}

//#include <cpp_redis/cpp_redis>
//#include <Windows.h>
//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <crtdbg.h>  // For _CrtSetReportMode
//#include <errno.h>
//#include "../../DBConnector/DBConnector.h"
//#include "../../NetworkLib/Parser.h"
//#include "../../NetworkLib/CrashDump.h"
//using namespace std;
//
//
//
//
//int main()
//{
//	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//	COORD cursorPos = { 10, 10 };
//	SetConsoleCursorPosition(hConsole, cursorPos);
//
//	for (;;) {
//		Sleep(1000);
//
//		CONSOLE_SCREEN_BUFFER_INFO csbi;
//		GetConsoleScreenBufferInfo(hConsole, &csbi);
//
//		int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
//		int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
//
//		//std::cout << "Console width: " << width << std::endl;
//		//std::cout << "Console height: " << height << std::endl;
//
//		for (int i = 0; i < height; i++) {
//			SetConsoleCursorPosition(hConsole, { 0, (short)i });
//			for (int j = 0; j < width; j++) {
//				printf("0");
//			}
//		}
//	}
//}
//
