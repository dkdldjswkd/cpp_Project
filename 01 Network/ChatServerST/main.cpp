#include <iostream>
#include <conio.h>
#include "ChatServerST.h"
#include "MonitoringClient.h"
#include "../../00 lib_jy/CrashDump.h"
#include "../../00 lib_jy/StringUtils.h"
#include "../../00 lib_jy/Profiler.h"
using namespace std;

void ServerMonitor(ChatServerST* p_chatServer, MonitoringClient* p_MonitorClient);

CrashDump dump;

int main() {
	// ä�� ����
	ChatServerST chattingServer("../ServerConfig.ini", "ChattingServer_Single");
	chattingServer.Start();

	// ����͸� Ŭ��
	MonitoringClient Monitoringclient("../ServerConfig.ini", "MonitoringClient", &chattingServer);
	Monitoringclient.Start();

	// �ܼ� ����͸� ������ ����
	ServerMonitor(&chattingServer, &Monitoringclient);
}

void ServerMonitor(ChatServerST* p_chatServer, MonitoringClient* p_MonitorClient) {
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
				p_MonitorClient->Stop();
			}
			else if (c == 'e' || c == 'E') {
				return;
			}
		}

		// ����͸� ������ ������Ʈ
		p_chatServer->NetServer::UpdateTPS();
		p_MonitorClient->NetClient::UpdateTPS();
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
				"Job Count            : %d                  \n"
				"Update TPS           : %d                  \n"
				"                                           \n"
				"[ MonitorClient ] -------------------------\n"
				"sendMsgTPS           : %u                  \n"
				"recvMsgTPS           : %u                  \n"
				"                                           \n"
				"[ Process ] -------------------------------\n"
				"CPU Usage            : %d                  \n"
				"Using Memory(MB)     : %d                  \n"
				"                                           \n"
				"[ Machine ] -------------------------------\n"
				"CPU Usage            : %d                  \n"
				"Using Non Memory(MB) : %d                  \n"
				"avail Memory(MB)     : %d                  \n"
				"recv Kbytes          : %d                  \n"
				"send Kbytes          : %d                  \n"
				,
				// ChatServer
				p_chatServer->GetAcceptTotal(),
				p_chatServer->GetAcceptTPS(),
				p_chatServer->GetRecvTPS(),
				p_chatServer->GetSendTPS(),
				// Contents
				p_chatServer->GetSessionCount(),
				p_chatServer->GetUserCount(),
				PacketBuffer::GetUseCount(),
				p_chatServer->GetJobPoolCount(),
				p_chatServer->GetUpdateTPS(),

				// MonitorClient
				p_MonitorClient->GetRecvTPS(),
				p_MonitorClient->GetSendTPS(),

				// Process
				p_MonitorClient->GetProcessCpuUsage(),
				p_MonitorClient->GetProcessUsingMemMb(),

				// Machine
				p_MonitorClient->GetMachineCpuUsage(),
				p_MonitorClient->GetMachineUsingNonMemMb(),
				p_MonitorClient->GetMachineAvailMemMb(),
				p_MonitorClient->GetMachineRecvKbytes(),
				p_MonitorClient->GetMachineSendKbytes()
			);
		}
	}
}