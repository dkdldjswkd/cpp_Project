#include <iostream>
#include <thread>
#include "MonitoringNetServer.h"
#include "MonitoringLanServer.h"
#include "../../00 lib_jy/CrashDump.h"
using namespace std;

void ConsoleMonitoring(MonitoringNetServer* net_server, MonitoringLanServer* lan_server);

CrashDump dump;

int main() {
	// ����͸� �ݼ���
	MonitoringNetServer monitoringNetServer("../ServerConfig.ini", "MonitoringNetServer");
	monitoringNetServer.Start();

	// ����͸� ������
	MonitoringLanServer monitoringLanServer("../ServerConfig.ini", "MonitoringLanServer", &monitoringNetServer);
	monitoringLanServer.Start();

	// �ܼ� ����͸�
	ConsoleMonitoring(&monitoringNetServer, &monitoringLanServer);
}

void ConsoleMonitoring(MonitoringNetServer* net_server, MonitoringLanServer* lan_server) {
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