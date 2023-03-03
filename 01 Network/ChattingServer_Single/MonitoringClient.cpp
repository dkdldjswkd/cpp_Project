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
	*p_packet << (WORD)en_PACKET_SS_MONITOR_LOGIN;
	*p_packet << (int)serverNo;
	SendPacket(p_packet);
	PacketBuffer::Free(p_packet);

	updateThread = thread([this] { UpdateFunc(); });
}

void MonitoringClient::OnRecv(PacketBuffer* contents_packet){
}

void MonitoringClient::OnDisconnect(){
}

void MonitoringClient::UpdateFunc() {
	time_t monitorTime;
	PacketBuffer* p_packet;
	for (;;) {
		Sleep(1000);
		time(&monitorTime);
		ProcessMonitor.UpdateCpuUsage();
		machineMonitor.UpdateCpuUsage();
		perfCounter.Update();

		// 에이전트 ChatServer 실행 여부 ON / OFF, 30
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN; // 데이터 항목
		*p_packet << (int)1; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 에이전트 ChatServer CPU 사용률, 31
		cpuUsage_chattingServer = ProcessMonitor.GetTotalCpuUsage();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU; // 데이터 항목
		*p_packet << (int)cpuUsage_chattingServer; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 에이전트 ChatServer 메모리 사용 MByte, 32
		usingMemoryMB_chattingServer = perfCounter.GetUserMemB() / 1024 / 1024;
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM; // 데이터 항목
		*p_packet << (int)usingMemoryMB_chattingServer; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 세션 수 (컨넥션 수), 33
		sessionCount_chattServer = localServer->Get_sessionCount();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SESSION; // 데이터 항목
		*p_packet << (int)sessionCount_chattServer; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 인증성공 사용자 수 (실제 접속자), 34
		userCount_chattServer = ((ChattingServer_Single*)localServer)->Get_playerCount();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PLAYER; // 데이터 항목
		*p_packet << (int)userCount_chattServer; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 UPDATE 스레드 초당 초리 횟수, 35
		updateTPS_chattServer = ((ChattingServer_Single*)localServer)->Get_updateTPS();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS; // 데이터 항목
		*p_packet << (int)updateTPS_chattServer; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 패킷풀 사용량, 36
		packetCount_chattingServer = PacketBuffer::Get_UseCount();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL; // 데이터 항목
		*p_packet << (int)packetCount_chattingServer; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 UPDATE MSG 풀 사용량, 37
		jobCount_chattServe = ((ChattingServer_Single*)localServer)->Get_JobQueueCount();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL; // 데이터 항목
		*p_packet << (int)jobCount_chattServe; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 CPU 전체 사용률, 40
		cpuUsage_machine = machineMonitor.GetTotalCpuUsage();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL; // 데이터 항목
		*p_packet << (int)cpuUsage_machine; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 논페이지 메모리 MByte, 41
		usingNonMemoryMB_machine = perfCounter.GetSysNonMemB() / 1024 / 1024;
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY; // 데이터 항목
		*p_packet << (int)usingNonMemoryMB_machine; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 네트워크 수신량 KByte, 42
		recvKbytes_machine = perfCounter.GetRecvBytes() / 1024;
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV; // 데이터 항목
		*p_packet << (int)recvKbytes_machine; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 네트워크 송신량 KByte, 43
		sendKbytes_machine = perfCounter.GetSendBytes() / 1024;
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND; // 데이터 항목
		*p_packet << (int)sendKbytes_machine; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 사용가능 메모리, 44
		availMemMB_machine = perfCounter.GetAvailMemMB();
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY; // 데이터 항목
		*p_packet << (int)availMemMB_machine; // 데이터 수치
		*p_packet << (int)monitorTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);
	}
}