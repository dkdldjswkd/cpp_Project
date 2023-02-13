#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "ChattingServer_Multi.h"
#include "../NetworkLib/CrashDump.h"
#include "../NetworkLib/Logger.h"
#pragma comment (lib, "Winmm.lib")

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

// CattingServer.cpp, Define 확인
void StartChattingServer_Multi(int maxThread, int releaseThread) {
	ChattingServer_Multi ChattingServer_Multi;
	ChattingServer_Multi.StartUp(NetworkArea::NET, INADDR_ANY, 12001, maxThread, releaseThread, true, 10000);
	printf("StartChattingServer Multi \n");

	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		printf("NetworkLib -------------------------------------------------- \n");
		printf("sessionCount    : %d \n", ChattingServer_Multi.Get_sessionCount());
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", ChattingServer_Multi.Get_acceptTotal());
		printf("acceptTPS       : %d \n", ChattingServer_Multi.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", ChattingServer_Multi.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", ChattingServer_Multi.Get_recvTPS());
		printf("ChattingServer-Multi ----------------------------------------- \n");
		printf("PlayerCount     : %d \n", ChattingServer_Multi.Get_playerCount());
		printf("PlayerPoolCount : %d \n", ChattingServer_Multi.Get_playerPoolCount());
		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n");
	}

	ChattingServer_Multi.CleanUp();
}

int main() {
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;

	StartChattingServer_Multi(maxThread, releaseThread);
}