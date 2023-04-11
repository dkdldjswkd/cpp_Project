#include <Windows.h>
#include <iostream>
#include <time.h>
#include <unordered_map>

using namespace std;

union MonitorKey {
	struct {
		short serverNo;
		short dataType;
	} tuple;
	DWORD key;
};

int main() {
	unordered_map<DWORD, int*> monitorDataMap;

	MonitorKey key;
	key.tuple.serverNo = 1;
	key.tuple.dataType = 2;

	int* p = new int;
	*p = 3;

	monitorDataMap.insert({ key.key, p });
	cout << *monitorDataMap.find(key.key)->second << endl;
	(*monitorDataMap.find(key.key)->second)--;
	cout << *monitorDataMap.find(key.key)->second << endl;
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