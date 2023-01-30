#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#include "ChattingServer.h"
#include "ChattingServerMulti.h"
#include "CrashDump.h"
#pragma comment (lib, "Winmm.lib")

// 모니터링
int sendQ_remain = 0;

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

// EchoServer.cpp, Define 확인
void StartEchoServer(int maxThread, int releaseThread) {
	EchoServer echo_server;
	echo_server.StartUp(NetworkArea::LAN, INADDR_ANY, 6000, maxThread, releaseThread, true, 10000);
	printf("StartEchoServer \n");

	for (;;) {
		Sleep(1000);
		echo_server.PrintTPS();
	}

	echo_server.CleanUp();
}

//// CattingServer.cpp, Define 확인
//void StartChattingServer(int maxThread, int releaseThread) {
//	ChattingServer	chatting_server;
//	chatting_server.StartUp(NetworkArea::NET, INADDR_ANY, 12001, maxThread, releaseThread, true, 10000);
//	printf("StartChattingServer \n");
//
//	for (;;) {
//		Sleep(1000);
//		chatting_server.PrintTPS();
//	}
//
//	chatting_server.CleanUp();
//}

// CattingServer.cpp, Define 확인
void StartChattingServerMulti(int maxThread, int releaseThread) {
	ChattingServerMulti	chattingServerMulti;
	chattingServerMulti.StartUp(NetworkArea::NET, INADDR_ANY, 12001, maxThread, releaseThread, true, 10000);
	printf("StartChattingServer Multi \n");

	for (;;) {
		Sleep(1000);
		chattingServerMulti.PrintTPS();
	}

	chattingServerMulti.CleanUp();
}

#define CHATTING
int main() {
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;

#ifdef CHATTING
	StartChattingServerMulti(maxThread, releaseThread);
#endif
#ifndef CHATTING
	StartEchoServer(maxThread, releaseThread);
#endif
}