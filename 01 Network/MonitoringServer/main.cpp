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
				// 1초 주기 모니터링
				Sleep(1000);
				system("cls");
				SetConsoleCursorPosition(h, { 0, 0 });

				// 콘솔 출력
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
						net_server->Get_sessionCount(),
						net_server->Get_acceptTotal(),
						net_server->Get_acceptTPS(),
						net_server->Get_sendTPS(),
						net_server->Get_recvTPS(),
						// lan server
						lan_server->Get_sessionCount(),
						lan_server->Get_acceptTotal(),
						lan_server->Get_acceptTPS(),
						lan_server->Get_sendTPS(),
						lan_server->Get_recvTPS()
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

	// 모니터링 넷서버
	MonitoringNetServer monitoringNetServer("../ServerConfig.ini", "MonitoringNetServer");
	monitoringNetServer.StartUp();

	// 모니터링 랜서버
	MonitoringLanServer monitoringLanServer("../ServerConfig.ini", "MonitoringLanServer", &monitoringNetServer);
	monitoringLanServer.StartUp();

	// 콘솔 모니터링 스레드 생성
	auto t = ConsoleMonitoring(&monitoringNetServer, &monitoringLanServer);

	Sleep(INFINITE);
}