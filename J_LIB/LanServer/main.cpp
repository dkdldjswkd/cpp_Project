#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#pragma comment (lib, 	"Winmm.lib");

#define IP INADDR_ANY


using namespace std;

int main() {
	//timeBeginPeriod(1);

	EchoServer echo_server;
	echo_server.StartUp(INADDR_ANY, 6000, 2, true, 10000);

	for (;;) {
		Sleep(1000);
		echo_server.Monitoring();
	}

	echo_server.CleanUp();
}