#include <iostream>
#include <thread>
#include "MonitoringNetServer.h"
#include "MonitoringLanServer.h"
#include "../../00 lib_jy/CrashDump.h"
using namespace std;

thread ConsoleMonitoring(MonitoringNetServer* net_server, MonitoringLanServer* lan_server) {
	thread monitor_thread([net_server, lan_server]
		{
			auto h = GetStdHandle(STD_OUTPUT_HANDLE);
			for (;;) {
				// 1�� �ֱ� ����͸�
				Sleep(1000);
				system("cls");
				SetConsoleCursorPosition(h, { 0, 0 });

				// �ܼ� ���
				{
					printf(
						"Total (Process : MonitoringServer)--------------\n"
						"PacketCount     : %d                            \n"
						"                                                \n"
						"Monitoring Net Lib -----------------------------\n"
						"sessionCount    : %d                            \n"
						"acceptTotal     : %d                            \n"
						"acceptTPS       : %d                            \n"
						"sendMsgTPS      : %d                            \n"
						"recvMsgTPS      : %d                            \n"
						"                                                \n"
						"Monitoring Lan Lib -----------------------------\n"
						"sessionCount    : %d                            \n"
						"acceptTotal     : %d                            \n"
						"acceptTPS       : %d                            \n"
						"sendMsgTPS      : %d                            \n"
						"recvMsgTPS      : %d                            \n"
						"					                             \n"
						,
						// total
						PacketBuffer::GetUseCount(),
						// net server
						net_server->GetSessionCount(),
						net_server->GetAcceptTotal(),
						net_server->GetAcceptTPS(),
						net_server->GetSendTPS(),
						net_server->GetRecvTPS(),
						// lan server
						lan_server->GetSessionCount(),
						lan_server->GetAcceptTotal(),
						lan_server->GetAcceptTPS(),
						lan_server->GetSendTPS(),
						lan_server->GetRecvTPS()
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

	// ����͸� �ݼ���
	MonitoringNetServer monitoringNetServer("../ServerConfig.ini", "MonitoringNetServer");
	monitoringNetServer.StartUp();

	// ����͸� ������
	MonitoringLanServer monitoringLanServer("../ServerConfig.ini", "MonitoringLanServer", &monitoringNetServer);
	monitoringLanServer.StartUp();

	// �ܼ� ����͸� ������ ����
	auto t = ConsoleMonitoring(&monitoringNetServer, &monitoringLanServer);

	Sleep(INFINITE);
}