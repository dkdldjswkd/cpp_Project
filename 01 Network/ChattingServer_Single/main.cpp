#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "ChattingServer_Single.h"
#include "../NetworkLib/CrashDump.h"
#include "../NetworkLib/Logger.h"
#pragma comment (lib, "Winmm.lib")
using namespace std;

#define IP INADDR_ANY

CrashDump dump;

void StartChattingServer_Single(int maxThread, int releaseThread) {
	ChattingServer_Single server;
	server.StartUp(NetworkArea::NET, INADDR_ANY, 12001, maxThread, releaseThread, true, 10000);
	printf("StartChattingServer Multi \n");

	for (;;) {
		// 1�� �ֱ� ����͸�
		Sleep(1000);
		printf("NetworkLib ---------------------------------------------------- \n");
		printf("sessionCount    : %d \n", server.Get_sessionCount());
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", server.Get_acceptTotal());
		printf("acceptTPS       : %d \n", server.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", server.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", server.Get_recvTPS());
		printf("					                                            \n");
		printf("ChattingServer-Single ----------------------------------------- \n");
		printf("PlayerCount     : %d \n", server.Get_playerCount());
		printf("PlayerPoolCount : %d \n", server.Get_playerPoolCount());
		printf("ó���� -------------------------------------------------------- \n");
		printf("JobPoolCount    : %d \n", server.Get_JobPoolCount());
		printf("JobQueueCount   : %d \n", server.Get_JobQueueCount());
		printf("UpdateTPS       : %d \n", server.Get_updateTPS());
		printf("����� -------------------------------------------------------- \n");
		printf("var             : %d \n", 0);
		printf("\n\n\n\n\n\n\n\n\n\n \n\n");
	}

	server.CleanUp();
}

int main() {
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;

	StartChattingServer_Single(maxThread, releaseThread);
}