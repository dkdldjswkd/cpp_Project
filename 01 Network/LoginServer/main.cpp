#include <iostream>
#include <thread>
#include <conio.h>
#include "LoginServer.h"
#include "../../00 lib_jy/CrashDump.h"
using namespace std;

void ConsoleMonitor(LoginServer* net_server);

CrashDump dump;

int main() {
	// 채팅 서버
	LoginServer loginServer("../ServerConfig.ini", "LoginServer");
	loginServer.Start();

	// 콘솔 모니터링 스레드 생성
	ConsoleMonitor(&loginServer);
}

void ConsoleMonitor(LoginServer* net_server) {
	auto h = GetStdHandle(STD_OUTPUT_HANDLE);
	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		system("cls");
		SetConsoleCursorPosition(h, { 0, 0 });

		// 서버 조작
		if (_kbhit()) {
			auto c = _getch();
			if (c == 'q' || c == 'Q') {
				//net_server->ServerStop();
			}
		}

		// 콘솔 출력
		{
			printf(
				"Process : LoginServer ---------------------\n"
				"LoginServer::NetLib -----------------------\n"
				"sessionCount    : %d                       \n"
				"PacketCount     : %d                       \n"
				"acceptTotal     : %d                       \n"
				"acceptTPS       : %d                       \n"
				"sendMsgTPS      : %d                       \n"
				"recvMsgTPS      : %d                       \n"
				"LoginServer -------------------------------\n"
				"PlayerCount     : %d                       \n"
				"PlayerPoolCount : %d                       \n"
				,
				// LoginServer lib
				net_server->GetSessionCount(),
				PacketBuffer::GetUseCount(),
				net_server->GetAcceptTotal(),
				net_server->GetAcceptTPS(),
				net_server->GetSendTPS(),
				net_server->GetRecvTPS(),
				// LoginServer
				net_server->GetUserCount(),
				net_server->GetUserPoolCount()
			);
		}
	}
}
