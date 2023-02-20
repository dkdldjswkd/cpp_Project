#include <iostream>
#include "ChattingServer_Single.h"
#include "../NetworkLib/CrashDump.h"
using namespace std;

void StartChattingServer_Single() {
	ChattingServer_Single server("../ServerConfig.ini", "ChattingServer_Single");
	server.StartUp();
	printf("StartChattingServer Single \n");

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
		printf("����� -------------------------------------------------------- \n"); extern int failCount;
		printf("failCount       : %d \n", failCount);
		printf("\n\n\n\n\n\n\n\n\n\n \n\n");
	}

	server.CleanUp();
}

int main() {
	static CrashDump dump;
	StartChattingServer_Single();
	Sleep(INFINITE);
}