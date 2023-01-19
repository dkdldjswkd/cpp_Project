#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#include "CrashDump.h"
#pragma comment (lib, 	"Winmm.lib")

int sendQ_remain = 0;

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

void StartEchoServer(int thread_num) {
	EchoServer echo_server;
	//DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session
	echo_server.StartUp(INADDR_ANY, 6000, thread_num, true, 10000);

	for (;;) {
		Sleep(1000);
		echo_server.PrintTPS();
	}

	echo_server.CleanUp();
}

int main() {
	int thread_num;
	printf("worker thread num >> ");
	cin >> thread_num;

	StartEchoServer(thread_num);
}