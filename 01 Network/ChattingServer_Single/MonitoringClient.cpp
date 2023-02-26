#include "MonitoringClient.h"
#include "MonitorProtocol.h"
#include <time.h>
using namespace std;

MonitoringClient::MonitoringClient(const char* systemFile, const char* client) : NetClient(systemFile, client) {
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

	updateThread = thread([this] {UpdateFunc(); });
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

		// ������Ʈ ChatServer ���� ���� ON / OFF
		{
			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN;
			*p_packet << (int)0;
			*p_packet << (int)t;
			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		//// ������Ʈ ChatServer CPU ����
		//{
		//	PacketBuffer* p_packet = PacketBuffer::Alloc();
		//	*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU;
		//	// cpu ����
		//	*p_packet << (int)0;
		//	*p_packet << (int)t;
		//	SendPacket(p_packet);
		//	PacketBuffer::Free(p_packet);
		//}
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