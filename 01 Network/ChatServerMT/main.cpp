#include <iostream>
#include <conio.h>
#include "ChatServerMT.h"
#include "../../00 lib_jy/CrashDump.h"
#include "../../00 lib_jy/StringUtils.h"
#include "../../00 lib_jy/Profiler.h"
using namespace std;

void ConsoleMonitor(ChatServerMT* p_chatServer);

CrashDump dump;

int main() {
	// ä�� ����
	ChatServerMT chatServer("../ServerConfig.ini", "ChattingServer_Multi");
	chatServer.Start();

	// �ܼ� ����͸� ������ ����
	ConsoleMonitor(&chatServer);
}

void ConsoleMonitor(ChatServerMT* p_chatServer) {
	auto h = GetStdHandle(STD_OUTPUT_HANDLE);
	for (;;) {
		// 1�� �ֱ� ����͸�
		Sleep(1000);
		system("cls");
		SetConsoleCursorPosition(h, { 0, 0 });

		// �������Ϸ� ���� out
		if (_kbhit()) {
			auto c = _getch();
			if (c == 'f' || c == 'F') {
				PRO_FILEOUT();
			}
			else if (c == 'r' || c == 'R') {
				PRO_RESET();
			}
			else if (c == 'q' || c == 'Q') {
				p_chatServer->Stop();
			}
			else if (c == 'e' || c == 'E') {
				return;
			}
		}

		// ����͸� ������ ������Ʈ
		p_chatServer->NetServer::UpdateTPS();
		p_chatServer->UpdateTPS();

		// �ܼ� ���
		{
			printf(
				"[ ChatServer ] ----------------------------\n"
				"acceptTotal          : %u                  \n"
				"acceptTPS            : %u                  \n"
				"recvMsgTPS           : %u                  \n"
				"sendMsgTPS           : %u                  \n"
				"[ Contents ] ------------------------------\n"
				"Session Count        : %d                  \n"
				"User Count           : %d                  \n"
				"Packet Count         : %d                  \n"
				// Update TPS		  : %d
				,
				// ChatServer
				p_chatServer->GetAcceptTotal(),
				p_chatServer->GetAcceptTPS(),
				p_chatServer->GetRecvTPS(),
				p_chatServer->GetSendTPS(),
				// Contents
				p_chatServer->GetSessionCount(),
				p_chatServer->GetUserCount(),
				PacketBuffer::GetUseCount()
				);
		}
	}
}