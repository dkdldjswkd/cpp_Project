#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#include "ChattingServer.h"
#include "CrashDump.h"
#pragma comment (lib, 	"Winmm.lib")

// 모니터링
int sendQ_remain = 0;

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

// EchoServer.cpp, Define 확인
void StartEchoServer(int thread_num) {
	EchoServer echo_server;
	//DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session
	echo_server.StartUp(NetworkArea::LAN, INADDR_ANY, 6000, thread_num, true, 10000);
	printf("StartEchoServer \n");

	for (;;) {
		Sleep(1000);
		echo_server.PrintTPS();
	}

	echo_server.CleanUp();
}

// CattingServer.cpp, Define 확인
void StartChattingServer(int thread_num) {
	ChattingServer	chatting_server;
	//DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session
	chatting_server.StartUp(NetworkArea::NET, INADDR_ANY, 12001, thread_num, true, 10000);
	printf("StartChattingServer \n");

	for (;;) {
		Sleep(1000);
		chatting_server.PrintTPS();
	}

	chatting_server.CleanUp();
}

//#define CHATTING
int main() {
	int thread_num;
	printf("worker thread num >> ");
	cin >> thread_num;

#ifdef CHATTING
	StartChattingServer(thread_num);
#endif
#ifndef CHATTING
	StartEchoServer(thread_num);
#endif
}