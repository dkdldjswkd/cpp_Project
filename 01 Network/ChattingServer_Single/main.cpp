#include <iostream>
#include <conio.h>
#include "ChattingServer_Single.h"
#include "MonitoringClient.h"
#include "../../00 lib_jy/CrashDump.h"
#include "../../00 lib_jy/StringUtils.h"
#include "../../00 lib_jy/Profiler.h"
using namespace std;

thread ConsoleMonitoring(ChattingServer_Single* net_server, MonitoringClient* net_client) {
	thread monitor_thread([net_server, net_client]
		{
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
				}

				// �ܼ� ���
				{
					printf(
						"Process : ChattingServer_Single------------\n"
						"                                           \n"
						"ChattingServer NerServer Lib --------------\n"
						"acceptTotal          : %u                  \n"
						"acceptTPS            : %u                  \n"
						"sendMsgTPS           : %u                  \n"
						"recvMsgTPS           : %u                  \n"
						"                                           \n"
						"MonitoringClient NetClient Lib ------------\n"
						"sendMsgTPS           : %u                  \n"
						"recvMsgTPS           : %u                  \n"
						"ChattingServer Status ---------------------\n"
						"CPU Usage(server)    : %d                  \n"
						"Using Memory(MB)     : %d                  \n"
						"Packet Count         : %d                  \n"
						"Session Count        : %d                  \n"
						"User Count           : %d                  \n"
						"Update TPS           : %d                  \n"
						"Job Count            : %d                  \n"
						"Machine Status ----------------------------\n"
						"CPU Usage(machine)   : %d                  \n"
						"Using Non Memory(MB) : %d                  \n"
						"recv Kbytes          : %d                  \n"
						"send Kbytes          : %d                  \n"
						"avail Memory(MB)     : %d                  \n"
						,
						// ChattingServer lib
						net_server->GetAcceptTotal(),
						net_server->GetAcceptTPS(),
						net_server->GetSendTPS(),
						net_server->GetRecvTPS(),
						// MonitoringClient lib
						net_client->Get_sendTPS(),
						net_client->Get_recvTPS(),
						// ChattingServer Status
						net_client->cpuUsage_chattingServer,
						net_client->usingMemoryMB_chattingServer,
						net_client->packetCount_chattingServer,
						net_client->sessionCount_chattServer,
						net_client->userCount_chattServer,
						net_client->updateTPS_chattServer,
						net_client->jobCount_chattServe,
						// Machine Status
						net_client->cpuUsage_machine,
						net_client->usingNonMemoryMB_machine,
						net_client->recvKbytes_machine,
						net_client->sendKbytes_machine,
						net_client->availMemMB_machine
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
	ChattingServer_Single chattingServer("../ServerConfig.ini", "ChattingServer_Single");
	chattingServer.StartUp();

	// ����͸� Ŭ��
	MonitoringClient Monitoringclient("../ServerConfig.ini", "MonitoringClient", &chattingServer);
	Monitoringclient.StartUp();

	// �ܼ� ����͸� ������ ����
	auto t = ConsoleMonitoring(&chattingServer, &Monitoringclient);

	Sleep(INFINITE);
}