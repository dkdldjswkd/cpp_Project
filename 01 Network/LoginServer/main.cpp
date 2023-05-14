#include <iostream>
#include <thread>
#include <conio.h>
#include "LoginServer.h"
#include "../../00 lib_jy/CrashDump.h"
#include "../../00 lib_jy/Profiler.h"
using namespace std;

void ConsoleMonitor(LoginServer* net_server);

CrashDump dump;

int main() {
	// 채팅 서버
	LoginServer loginServer("../ServerConfig.ini", "LoginServer");
	loginServer.Start();

	// 콘솔 모니터
	ConsoleMonitor(&loginServer);
}

void ConsoleMonitor(LoginServer* net_server){
	auto h = GetStdHandle(STD_OUTPUT_HANDLE);
	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		system("cls");
		SetConsoleCursorPosition(h, { 0, 0 });

		// 프로파일러 파일 out
		if (_kbhit()) {
			auto c = _getch();
			if (c == 'f' || c == 'F') {
				PRO_FILEOUT();
			}
			else if (c == 'r' || c == 'R') {
				PRO_RESET();
			}
			else if (c == 'q' || c == 'Q') {
				net_server->Stop();
			}
			else if (c == 'e' || c == 'E') {
				return;
			}
		}

		// 모니터링 데이터 업데이트
		net_server->UpdateTPS();

		// 콘솔 출력
		{
			printf(
				"[ LoginServer ] ---------------------------\n"
				"acceptTotal          : %u                  \n"
				"acceptTPS            : %u                  \n"
				"recvMsgTPS           : %u                  \n"
				"sendMsgTPS           : %u                  \n"
				"[ Contents ] ------------------------------\n"
				"Session Count        : %d                  \n"
				"User Count           : %d                  \n"
				"Packet Count         : %d                  \n"
				"User Pool Count      : %d                  \n"
				,
				// ChatServer
				net_server->GetAcceptTotal(),
				net_server->GetAcceptTPS(),
				net_server->GetRecvTPS(),
				net_server->GetSendTPS(),
				// Contents
				net_server->GetSessionCount(),
				net_server->GetUserCount(),
				PacketBuffer::GetUseCount(),
				net_server->GetUserPoolCount()
				);
		}
	}
}