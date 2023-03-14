#include <stdio.h>
#include <time.h>

int main()
{
    time_t currentTime;

    while (1) {
        time(&currentTime);
        printf("Current time: %llu", currentTime);
    }

    return 0;
}

//#include <Windows.h>
//#include <cstdio>
//
//SYSTEMTIME t;
//int main() {
//	//GetLocalTime(&t);
//	//t.wHour += 9;
//	//if (24 <= t.wHour) {
//	//	t.wHour -= 24;
//	//	t.wDay += 1;
//	//}
//	//SetLocalTime(&t);
//
//	for (;;) {
//		Sleep(1000);
//		char LogTime[64];
//		GetLocalTime(&t);
//
//		sprintf_s(LogTime, sizeof(LogTime), "%04d-%02d-%02d %02d:%02d:%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
//
//		printf("%s\n", LogTime);
//	}
//
//	return 0;
//} 