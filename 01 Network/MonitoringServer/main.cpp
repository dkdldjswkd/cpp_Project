#include <iostream>
#include <thread>
#include "MonitoringLanServer.h"
#include "MonitoringNetServer.h"
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

				printf(	"Total ------------------------------------------------------------\n"
						"PacketCount     : %d                                              \n"
						"                                                                  \n"
						"Monitoring Net Lib -----------------------------------------------\n"
						"sessionCount    : %d                                              \n"
						"acceptTotal     : %d                                              \n"
						"acceptTPS       : %d                                              \n"
						"sendMsgTPS      : %d                                              \n"
						"recvMsgTPS      : %d                                              \n"
						"                                                                  \n"
						"Monitoring Lan Lib -----------------------------------------------\n"
						"sessionCount    : %d                                              \n"
						"acceptTotal     : %d                                              \n"
						"acceptTPS       : %d                                              \n"
						"sendMsgTPS      : %d                                              \n"
						"recvMsgTPS      : %d                                              \n"
						"					                                               \n",
					PacketBuffer::Get_UseCount(),	// total
					net_server->Get_sessionCount(), // net server
					net_server->Get_acceptTotal(),
					net_server->Get_acceptTPS(),
					net_server->Get_sendTPS(),
					net_server->Get_recvTPS(),
					lan_server->Get_sessionCount(), // lan server
					lan_server->Get_acceptTotal(),
					lan_server->Get_acceptTPS(),
					lan_server->Get_sendTPS(),
					lan_server->Get_recvTPS()
					);
			}
		}
	);
	return monitor_thread;
}

int main() {
	static CrashDump dump;

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