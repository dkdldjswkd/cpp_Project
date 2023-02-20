#include <iostream>
#include "LoginServer.h"
#include "../NetworkLib/CrashDump.h"
using namespace std;

void StartLoginServer() {
	LoginServer loginServer("../ServerConfig.ini", "LoginServer");
	loginServer.StartUp();
	printf("StartLoginServer \n");

	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		printf("NetworkLib -------------------------------------------------- \n");
		printf("sessionCount    : %d \n", loginServer.Get_sessionCount());
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", loginServer.Get_acceptTotal());
		printf("acceptTPS       : %d \n", loginServer.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", loginServer.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", loginServer.Get_recvTPS());
		printf("LoginServer ------------------------------------------------- \n");
		printf("PlayerCount     : %d \n", loginServer.Get_playerCount());
		printf("PlayerPoolCount : %d \n", loginServer.Get_playerPoolCount());
		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n");
	}
	loginServer.CleanUp();
}

int main() {
	static CrashDump dump;
	StartLoginServer();
}