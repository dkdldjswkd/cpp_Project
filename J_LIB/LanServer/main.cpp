#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#include "CrashDump.h"
#pragma comment (lib, 	"Winmm.lib");

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

void StartEchoServer() {
	EchoServer echo_server;
	//DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session
	echo_server.StartUp(INADDR_ANY, 6000, 2, true, 10000);

	for (;;) {
		Sleep(1000);
		echo_server.PrintTPS();
	}

	echo_server.CleanUp();
}

int main() {
	//timeBeginPeriod(1);
	StartEchoServer();
}