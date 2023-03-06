#include "ChattingServer_Single.h"
#include "MonitoringClient.h"
#include "MonitorProtocol.h"
#include <time.h>
using namespace std;

MonitoringClient::MonitoringClient(const char* systemFile, const char* client, NetServer* localServer) : NetClient(systemFile, client), localServer(localServer) {
	parser.GetValue(client, "CHATT_SERVER_NO", &serverNo);
	updateThread = thread([this] { Report_to_monitoringServer(); });
}

MonitoringClient::~MonitoringClient() {
}

void MonitoringClient::OnConnect() {
	isConnect = true;

	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_SS_MONITOR_LOGIN;
	*p_packet << (int)serverNo;
	SendPacket(p_packet);
	PacketBuffer::Free(p_packet);
}

void MonitoringClient::OnRecv(PacketBuffer* contents_packet) {
}

void MonitoringClient::OnDisconnect() {
	isConnect = false;
}

void MonitoringClient::Report_to_monitoringServer() {
	for (;;) {
		Sleep(1000);
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
		*p_packet << (int)cpuUsage_chattingServer; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������Ʈ ChatServer �޸� ��� MByte, 32
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM; // ������ �׸�
		*p_packet << (int)usingMemoryMB_chattingServer; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� ���� �� (���ؼ� ��), 33
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SESSION; // ������ �׸�
		*p_packet << (int)sessionCount_chattServer; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� �������� ����� �� (���� ������), 34
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PLAYER; // ������ �׸�
		*p_packet << (int)userCount_chattServer; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� UPDATE ������ �ʴ� �ʸ� Ƚ��, 35
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS; // ������ �׸�
		*p_packet << (int)updateTPS_chattServer; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� ��ŶǮ ��뷮, 36
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL; // ������ �׸�
		*p_packet << (int)packetCount_chattingServer; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ä�ü��� UPDATE MSG Ǯ ��뷮, 37
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL; // ������ �׸�
		*p_packet << (int)jobCount_chattServe; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� CPU ��ü ����, 40
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL; // ������ �׸�
		*p_packet << (int)cpuUsage_machine; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� �������� �޸� MByte, 41
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY; // ������ �׸�
		*p_packet << (int)usingNonMemoryMB_machine; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� ��Ʈ��ũ ���ŷ� KByte, 42
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV; // ������ �׸�
		*p_packet << (int)recvKbytes_machine; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� ��Ʈ��ũ �۽ŷ� KByte, 43
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND; // ������ �׸�
		*p_packet << (int)sendKbytes_machine; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// ������ǻ�� ��밡�� �޸�, 44
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY; // ������ �׸�
		*p_packet << (int)availMemMB_machine; // ������ ��ġ
		*p_packet << (int)lastUpdateTime; // ���� �ð�
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);
	}
}

void MonitoringClient::UpdateData() {
	time(&lastUpdateTime);
	ProcessMonitor.UpdateCpuUsage();
	machineMonitor.UpdateCpuUsage();
	perfCounter.Update();

	cpuUsage_chattingServer = ProcessMonitor.GetTotalCpuUsage();
	usingMemoryMB_chattingServer = perfCounter.GetUserMemB() / 1024 / 1024;
	sessionCount_chattServer = localServer->Get_sessionCount();
	userCount_chattServer = ((ChattingServer_Single*)localServer)->Get_playerCount();
	updateTPS_chattServer = ((ChattingServer_Single*)localServer)->Get_updateTPS();
	packetCount_chattingServer = PacketBuffer::Get_UseCount();
	jobCount_chattServe = ((ChattingServer_Single*)localServer)->Get_JobQueueCount();
	cpuUsage_machine = machineMonitor.GetTotalCpuUsage();
	usingNonMemoryMB_machine = perfCounter.GetSysNonMemB() / 1024 / 1024;
	recvKbytes_machine = perfCounter.GetRecvBytes() / 1024;
	sendKbytes_machine = perfCounter.GetSendBytes() / 1024;
	availMemMB_machine = perfCounter.GetAvailMemMB();
}