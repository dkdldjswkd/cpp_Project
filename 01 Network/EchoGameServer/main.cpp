#include <iostream>
#include "EchoGameServer.h"
#include "../NetworkLib/CrashDump.h"
using namespace std;

// EchoServer.cpp, Define 확인
void StartEchoGameServer() {
	EchoGameServer server("../ServerConfig.ini", "EchoServer");
	server.StartUp();
	printf("Start EchoGameServer\n");

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
	static CrashDump dump;
	StartEchoGameServer();
	Sleep(INFINITE);
}