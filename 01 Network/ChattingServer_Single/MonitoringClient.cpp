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

		// 에이전트 ChatServer 실행 여부 ON / OFF
		{
			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN;
			*p_packet << (int)0;
			*p_packet << (int)t;
			SendPacket(p_packet);
			PacketBuffer::Free(p_packet);
		}

		//// 에이전트 ChatServer CPU 사용률
		//{
		//	PacketBuffer* p_packet = PacketBuffer::Alloc();
		//	*p_packet << dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU;
		//	// cpu 사용률
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
//		BYTE	DataType				// 모니터링 데이터 Type 하단 Define 됨.
//		int		DataValue				// 해당 데이터 수치.
//		int		TimeStamp				// 해당 데이터를 얻은 시간 TIMESTAMP  (time() 함수)
//										// 본래 time 함수는 time_t 타입변수이나 64bit 로 낭비스러우니
//										// int 로 캐스팅하여 전송. 그래서 2038년 까지만 사용가능
//	}
//
//------------------------------------------------------------
//en_PACKET_SS_MONITOR_DATA_UPDATE