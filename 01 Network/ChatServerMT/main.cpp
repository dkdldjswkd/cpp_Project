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
	// 채팅 서버
	ChatServerMT chatServer("../ServerConfig.ini", "ChattingServer_Multi");
	chatServer.Start();

	// 콘솔 모니터링 스레드 생성
	ConsoleMonitor(&chatServer);
}

void ConsoleMonitor(ChatServerMT* p_chatServer) {
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
				p_chatServer->Stop();
			}
			else if (c == 'e' || c == 'E') {
				return;
			}
		}

		// 모니터링 데이터 업데이트
		p_chatServer->NetServer::UpdateTPS();
		p_chatServer->UpdateTPS();

		// 콘솔 출력
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