#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;

char GetRandomCharacter() {
	return 'a' + (rand() % 26);
}

void GetRandomString(char* buf, int len) {
	buf[0] = 0;
	auto max_len = (rand() % len) + 1;
	printf("%d \n", max_len);
	for (int i = 0; i < max_len; i++) {
#pragma warning(suppress : 4996)
		sprintf(buf+i, "%c", GetRandomCharacter());
	}
	buf[max_len] = 0;
}

char buf[256];
int main() {
	for (;;) {
		GetRandomString(buf, 255);

		cout << buf << endl;
		cout << strlen(buf) << endl;
	}

}