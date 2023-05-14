#include "ChatServerMT.h"
#include "ChatServerMTMonitorClient.h"
#include "MonitorProtocol.h"
#include <time.h>
using namespace std;

ChatServerMTMonitorClient::ChatServerMTMonitorClient(const char* systemFile, const char* client, ChatServerMT* localServer) : NetClient(systemFile, client), p_chatServer(localServer) {
	parser.GetValue(client, "CHATT_SERVER_NO", &serverNo);
	updateThread = thread([this] { ReportToMonitoringServer(); });
}

ChatServerMTMonitorClient::~ChatServerMTMonitorClient() {
}

void ChatServerMTMonitorClient::OnDisconnect() {
	isConnect = false;
}

void ChatServerMTMonitorClient::OnClientStop(){
	updateRun = false;
	if (updateThread.joinable()) {
		updateThread.join();
	}
}

void ChatServerMTMonitorClient::OnConnect() {
	isConnect = true;

	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_SS_MONITOR_LOGIN;
	*p_packet << (int)serverNo;
	SendPacket(p_packet);
	PacketBuffer::Free(p_packet);
}

void ChatServerMTMonitorClient::UpdateData() {
	time(&lastUpdateTime);
	ProcessMonitor.UpdateCpuUsage();
	machineMonitor.UpdateCpuUsage();
	perfCounter.Update();

	// Process
	processCpuUsage = ProcessMonitor.GetTotalCpuUsage();
	processUsingMemMb = perfCounter.GetUserMemB() / 1024 / 1024;

	// Machine
	machineCpuUsage = machineMonitor.GetTotalCpuUsage();
	machineUsingNonMemMb = perfCounter.GetSysNonMemB() / 1024 / 1024;
	machineRecvKbytes = perfCounter.GetRecvBytes() / 1024;
	machineSendKbytes = perfCounter.GetSendBytes() / 1024;
	machineAvailMemMb = perfCounter.GetAvailMemMB();
}

void ChatServerMTMonitorClient::ReportToMonitoringServer() {
	for (;;) {
		Sleep(1000);
		if (!updateRun)
			break;

		UpdateData();
		// ����Ǿ����� �ʴٸ�, ������ ���Ÿ�
		if (!isConnect)
			continue;

		// ������Ʈ ChatServer ���� ���� ON / OFF, 30
		PacketBuffer* p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN; // ������ �׸�
		*p_packet << (int)1; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������Ʈ ChatServer CPU ����, 31
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU; // ������ �׸�
		*p_packet << (int)processCpuUsage; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������Ʈ ChatServer �޸� ��� MByte, 32
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM; // ������ �׸�
		*p_packet << (int)processUsingMemMb; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� ���� �� (���ؼ� ��), 33
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SESSION; // ������ �׸�
		*p_packet << (int)p_chatServer->GetSessionCount(); // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� �������� ����� �� (���� ������), 34
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PLAYER; // ������ �׸�
		*p_packet << (int)p_chatServer->GetUserCount(); // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� UPDATE ������ �ʴ� �ʸ� Ƚ��, 35
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS; // ������ �׸�
		*p_packet << (int)p_chatServer->GetUpdateTPS(); // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� ��ŶǮ ��뷮, 36
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL; // ������ �׸�
		*p_packet << (int)PacketBuffer::GetUseCount(); // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� CPU ��ü ����, 40
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL; // ������ �׸�
		*p_packet << (int)machineCpuUsage; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� �������� �޸� MByte, 41
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY; // ������ �׸�
		*p_packet << (int)machineUsingNonMemMb; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� ��Ʈ��ũ ���ŷ� KByte, 42
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV; // ������ �׸�
		*p_packet << (int)machineRecvKbytes; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� ��Ʈ��ũ �۽ŷ� KByte, 43
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND; // ������ �׸�
		*p_packet << (int)machineSendKbytes; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� ��밡�� �޸�, 44
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY; // ������ �׸�
		*p_packet << (int)machineAvailMemMb; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);
	}
}