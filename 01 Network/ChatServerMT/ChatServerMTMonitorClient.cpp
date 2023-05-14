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
		// 연결되어있지 않다면, 데이터 갱신만
		if (!isConnect)
			continue;

		// 에이전트 ChatServer 실행 여부 ON / OFF, 30
		PacketBuffer* p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN; // 데이터 항목
		*p_packet << (int)1; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 에이전트 ChatServer CPU 사용률, 31
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU; // 데이터 항목
		*p_packet << (int)processCpuUsage; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 에이전트 ChatServer 메모리 사용 MByte, 32
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM; // 데이터 항목
		*p_packet << (int)processUsingMemMb; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 세션 수 (컨넥션 수), 33
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_SESSION; // 데이터 항목
		*p_packet << (int)p_chatServer->GetSessionCount(); // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 인증성공 사용자 수 (실제 접속자), 34
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PLAYER; // 데이터 항목
		*p_packet << (int)p_chatServer->GetUserCount(); // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 UPDATE 스레드 초당 초리 횟수, 35
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS; // 데이터 항목
		*p_packet << (int)p_chatServer->GetUpdateTPS(); // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 채팅서버 패킷풀 사용량, 36
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL; // 데이터 항목
		*p_packet << (int)PacketBuffer::GetUseCount(); // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 CPU 전체 사용률, 40
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL; // 데이터 항목
		*p_packet << (int)machineCpuUsage; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 논페이지 메모리 MByte, 41
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY; // 데이터 항목
		*p_packet << (int)machineUsingNonMemMb; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 네트워크 수신량 KByte, 42
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV; // 데이터 항목
		*p_packet << (int)machineRecvKbytes; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 네트워크 송신량 KByte, 43
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND; // 데이터 항목
		*p_packet << (int)machineSendKbytes; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);

		// 서버컴퓨터 사용가능 메모리, 44
		p_packet = PacketBuffer::Alloc();
		*p_packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE; // 패킷 타입
		*p_packet << (BYTE)dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY; // 데이터 항목
		*p_packet << (int)machineAvailMemMb; // 데이터 수치
		*p_packet << (int)lastUpdateTime; // 측정 시간
		SendPacket(p_packet);
		PacketBuffer::Free(p_packet);
	}
}