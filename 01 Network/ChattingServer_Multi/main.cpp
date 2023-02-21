#include <iostream>
#include "ChattingServer_Multi.h"
#include "../NetworkLib/CrashDump.h"
using namespace std;

// CattingServer.cpp, Define 확인
void StartChattingServer_Multi() {
	ChattingServer_Multi server("../ServerConfig.ini", "ChattingServer_Multi");
	server.StartUp();
	printf("StartChattingServer Multi \n");

	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		printf("NetworkLib -------------------------------------------------- \n");
		printf("sessionCount    : %d \n", server.Get_sessionCount());
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", server.Get_acceptTotal());
		printf("acceptTPS       : %d \n", server.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", server.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", server.Get_recvTPS());
		printf("ChattingServer-Multi ----------------------------------------- \n");
		printf("PlayerCount     : %d \n", server.Get_playerCount());
		printf("PlayerPoolCount : %d \n", server.Get_playerPoolCount());
		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n");
	}

	server.CleanUp();
}

int main() {
	static CrashDump dump;
	StartChattingServer_Multi();
	Sleep(INFINITE);
}