#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#include "../NetworkLib/CrashDump.h"
#include "../NetworkLib/Logger.h"
#pragma comment (lib, "Winmm.lib")

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

// EchoServer.cpp, Define 확인
void StartEchoServer(int maxThread, int releaseThread) {
	EchoServer server;
	server.StartUp(NetworkArea::LAN, INADDR_ANY, 6000, maxThread, releaseThread, true, 10000, 10000, 10000);
	printf("StartEchoServer \n");

	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		printf("NetworkLib ---------------------------------------------------- \n");
		printf("sessionCount    : %d \n", server.Get_sessionCount()); 
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", server.Get_acceptTotal());
		printf("acceptTPS       : %d \n", server.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", server.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", server.Get_recvTPS());
		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n \n\n");
	}

	server.CleanUp();
}

int main() {
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;

	StartEchoServer(maxThread, releaseThread);
}