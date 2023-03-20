#include <iostream>
#include <thread>
#include <conio.h>
#include "LoginServer.h"
#include "../../00 lib_jy/CrashDump.h"
using namespace std;

thread ConsoleMonitoring(LoginServer* net_server) {
	thread monitor_thread([net_server]
		{
			auto h = GetStdHandle(STD_OUTPUT_HANDLE);
			for (;;) {
				// 1�� �ֱ� ����͸�
				Sleep(1000);
				system("cls");
				SetConsoleCursorPosition(h, { 0, 0 });

				// ���� ����
				if (_kbhit()) {
					auto c = _getch();
					if (c == 'q' || c == 'Q') {
						//net_server->ServerStop();
					}
				}

				// �ܼ� ���
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
						net_server->GetPlayerCount(),
						net_server->GetPlayerPoolCount()
					);
				}
			}
		}
	);
	return monitor_thread;
}

int main() {
	static CrashDump dump;
	SMALL_RECT rect = { 0,0,45,40 };
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

	// ä�� ����
	LoginServer loginServer("../ServerConfig.ini", "LoginServer");
	loginServer.Start();

	// �ܼ� ����͸� ������ ����
	auto t = ConsoleMonitoring(&loginServer);

	Sleep(INFINITE);
}