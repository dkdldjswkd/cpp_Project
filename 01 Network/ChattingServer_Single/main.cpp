#include <iostream>
#include "ChattingServer_Single.h"
#include "MonitoringClient.h"
#include "../../00 lib_jy/CrashDump.h"
#include "../../00 lib_jy/StringUtils.h"
using namespace std;

void f() { cout << 1 << endl; }

thread chatting_monitor(ChattingServer_Single* server) {
	thread monitor_thread([server]
		{
			system("cls");
			auto h = GetStdHandle(STD_OUTPUT_HANDLE);
			for (;;) {
				// 1�� �ֱ� ����͸�
				Sleep(1000);
				SetConsoleCursorPosition(h, { 0, 0 });
				printf(	"NetworkLib ----------------------------------------------------\n"
						"sessionCount    : %d                                           \n"
						"PacketCount     : %d                                           \n"
						"acceptTotal     : %d                                           \n"
						"acceptTPS       : %d                                           \n"
						"sendMsgTPS      : %d                                           \n"
						"recvMsgTPS      : %d                                           \n"
						"					                                            \n"
						"ChattingServer-Single -----------------------------------------\n"
						"PlayerCount     : %d                                           \n"
						"PlayerPoolCount : %d                                           \n"
						"ó���� --------------------------------------------------------\n"
						"JobPoolCount    : %d                                           \n"
						"JobQueueCount   : %d                                           \n"
						"UpdateTPS       : %d                                           \n"
						"����� --------------------------------------------------------\n"
						"var       : %d                                                 \n",
						server->Get_sessionCount(),
						PacketBuffer::Get_UseCount(),
						server->Get_acceptTotal(),
						server->Get_acceptTPS(),
						server->Get_sendTPS(),
						server->Get_recvTPS(),
						server->Get_playerCount(),
						server->Get_playerPoolCount(),
						server->Get_JobPoolCount(),
						server->Get_JobQueueCount(),
						server->Get_updateTPS(),
						0);
			}
		}
	);
	return monitor_thread;
}

int main() {
	static CrashDump dump;

	// ä�ü��� & �ܼ� ����͸� ������ ����
	ChattingServer_Single server("../ServerConfig.ini", "ChattingServer_Single");
	thread chatt_monitor = chatting_monitor(&server);
	server.StartUp();

	//// ����͸� Ŭ�� ����
	//MonitoringClient client("../ServerConfig.ini", "MonitoringClient");
	//client.StartUp();

	Sleep(INFINITE);
}