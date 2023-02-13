#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "../NetworkLib/CrashDump.h"
#include "../NetworkLib/Logger.h"
#include "LoginServer.h"
#pragma comment (lib, "Winmm.lib")
using namespace std;

#define IP			INADDR_ANY
#define PORT		30000
#define MAX_SESSION 10000

#define DB_IP		"127.0.0.1"
#define DB_PORT		3306
#define DB_ID		"root"
#define DB_PASSWORD "password"
#define DB_SCHEMA	"password"
#define DB_LOGTIME	200

void StartLoginServer(int maxThread, int releaseThread) {
	LoginServer loginServer(DB_IP, DB_PORT, DB_ID, DB_PASSWORD, DB_SCHEMA);
	loginServer.StartUp(NetworkArea::NET, IP, PORT, maxThread, releaseThread, true, MAX_SESSION);
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
		printf("var             : %d \n", 0);
		printf("var             : %d \n", 0);
		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n");
	}
	loginServer.CleanUp();
}

int main() {
	static CrashDump dump;
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;
	StartLoginServer(maxThread, releaseThread);
}