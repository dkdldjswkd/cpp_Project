#include "ChattingServer_Single.h"
#include "MonitoringClient.h"
#include "MonitorProtocol.h"
#include <time.h>
using namespace std;

MonitoringClient::MonitoringClient(const char* systemFile, const char* client, NetServer* localServer) : NetClient(systemFile, client), localServer(localServer) {
	parser.GetValue(client, "CHATT_SERVER_NO", &serverNo);
}

MonitoringClient::~MonitoringClient() {
}

void MonitoringClient::OnConnect() {
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << en_PACKET_SS_MONITOR_LOGIN;
	*p_packet << serverNo;
	SendPacket(p_packet);
	PacketBuffer::Free(p_packet);

	updateThread = thread([this] { UpdateFunc(); });
}

void MonitoringClient::OnRecv(PacketBuffer* contents_packet){
}

void MonitoringClient::OnDisconnect(){
}

void MonitoringClient::UpdateFunc(){
	for (;;) {
		Sleep(1000);
		time_t t;
		time(&t);
		PacketBuffer* p_packet;

		// ������Ʈ ChatServer ���� ���� ON / OFF, 30
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN; // ������ �׸�
			*p_packet << (int)1; // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������Ʈ ChatServer CPU ����, 31
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU; // ������ �׸�
			*p_packet << (int)ProcessMonitor.GetTotalCpuUsage(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������Ʈ ChatServer �޸� ��� MByte, 32
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM; // ������ �׸�
			*p_packet << (int)perfCounter.GetUserMem(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������Ʈ ChatServer �޸� ��� MByte, 32
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM; // ������ �׸�
			*p_packet << (int)perfCounter.GetUserMem(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ä�ü��� ���� �� (���ؼ� ��), 33
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SESSION; // ������ �׸�
			*p_packet << (int)localServer->Get_sessionCount(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ä�ü��� �������� ����� �� (���� ������), 34
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_PLAYER; // ������ �׸�
			*p_packet << (int)((ChattingServer_Single*)localServer)->Get_playerCount(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ä�ü��� UPDATE ������ �ʴ� �ʸ� Ƚ��, 35
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS; // ������ �׸�
			*p_packet << (int)((ChattingServer_Single*)localServer)->Get_updateTPS(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ä�ü��� ��ŶǮ ��뷮, 36
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL; // ������ �׸�
			*p_packet << (int)PacketBuffer::Get_UseCount(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ä�ü��� UPDATE MSG Ǯ ��뷮, 37
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL; // ������ �׸�
			*p_packet << (int)((ChattingServer_Single*)localServer)->Get_JobQueueCount(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������ǻ�� CPU ��ü ����, 40
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL; // ������ �׸�
			*p_packet << (int)machineMonitor.GetTotalCpuUsage(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������ǻ�� �������� �޸� MByte, 41
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY; // ������ �׸�
			*p_packet << (int)perfCounter.GetSysNonMem(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������ǻ�� ��Ʈ��ũ ���ŷ� KByte, 42
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV; // ������ �׸�
			*p_packet << (int)(perfCounter.GetRecvBytes() / 1024); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������ǻ�� ��Ʈ��ũ �۽ŷ� KByte, 43
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND; // ������ �׸�
			*p_packet << (int)(perfCounter.GetSendBytes() / 1024); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		// ������ǻ�� ��밡�� �޸�, 44
		{
			p_packet = PacketBuffer::Alloc();
			*p_packet << en_PACKET_SS_MONITOR_DATA_UPDATE; // ��Ŷ Ÿ��

			*p_packet << dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY; // ������ �׸�
			*p_packet << (int)perfCounter.GetAvailMem(); // ������ ��ġ
			*p_packet << (int)t; // ���� �ð�

			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}
	}
}

//	{
//		WORD	Type
//
//		BYTE	DataType				// ����͸� ������ Type �ϴ� Define ��.
//		int		DataValue				// �ش� ������ ��ġ.
//		int		TimeStamp				// �ش� �����͸� ���� �ð� TIMESTAMP  (time() �Լ�)
//										// ���� time �Լ��� time_t Ÿ�Ժ����̳� 64bit �� ���񽺷����
//										// int �� ĳ�����Ͽ� ����. �׷��� 2038�� ������ ��밡��
//	}
//
//------------------------------------------------------------
//en_PACKET_SS_MONITOR_DATA_UPDATE