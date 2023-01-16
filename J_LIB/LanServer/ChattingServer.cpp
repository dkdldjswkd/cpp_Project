#include "ChattingServer.h"
#include "CommonProtocol.h"
using namespace J_LIB;
using namespace std;

#define MAX_GQCS_DECUEING 100

ChattingServer::ChattingServer() {
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (h_iocp == INVALID_HANDLE_VALUE) CRASH();
	updateThread = thread([this]() {UpdateFunc(); });
	// 하트비트 스레드 생성
}

ChattingServer::~ChattingServer(){
}

bool ChattingServer::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void ChattingServer::OnClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc();
	p_player->Set(session_id);
	player_map.insert({ session_id, p_player });
	return;
}

void ChattingServer::OnClientLeave(SESSION_ID session_id) {
	// player map 에서 검색
	// player map 에서 player 삭제
	// free (player)
}

void ChattingServer::HeartbeatFunc() {
	for (;;) {
		// Sleep(10초)
		// curtime = timegettime();
		// 
		// Player map 순회
		// 
		// if (curtime - 세션 마지막 통신) > 40 ) // 로그인 시
		//		Disconnect Player

		// if (curtime - 세션 마지막 통신) > 1 ) // 비 로그인 시
		//		Disconnect Player
	}
}

void ChattingServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	// JOB.id = id;
	// JOB.packet = packet;
	// packet.refCount++;

	// PQCS(&JOB), 채팅 서버 완료 큐 (IOCP) 큐잉
}

void ChattingServer::UpdateFunc() {
	for (;;) {
		OVERLAPPED_ENTRY dequeueEntry[MAX_GQCS_DECUEING];
		LONG dequeueSize;

		BOOL ret_GQCS = GetQueuedCompletionStatusEx(h_iocp, dequeueEntry, MAX_GQCS_DECUEING, (PULONG)&dequeueSize, INFINITE, FALSE);

		for (int i = 0; i < dequeueSize; i++) {
			// JOB* job = dequeueEntry[i].lpCompletionKey;
			// if (job == nullptr) 서버 종료

			// job->id 로 player* 검색
			// job->packet >> packet_type;
			// ProcPacket(player*, packet_type, job->packet);
			// Free( job->packet )
		}
	}
}

bool ChattingServer::ProcPacket(Player* p_player, WORD type, PacketBuffer* cs_contentsPacket) {
	switch (type) {
		case en_PACKET_CS_CHAT_REQ_LOGIN: 
			return ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(p_player, cs_contentsPacket);
		
		// ...

		default:
			break;
	}

	return false;
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(Player* p_player, PacketBuffer* cs_contentsPacket){
	// cs_contentsPacket >> AccountNo, ID, NickName, SessionKey
	// 인증토큰 == true
	//		ID, Nickname Set
	//		로그인 응답 패킷 Send
	return true;
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket){
	// 로그인 여부 체크 -> 로그인 하지 않은 세션 대상 Disconnect
	// Sector X,Y 좌표 확인 (-1 or 섹터 초과 or Sector 좌표 점프) Disconnect
	// 플레이어 Sector 최신화
	// 섹터 이동결과 패킷 Send
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket){
	// 로그인 여부 체크 -> 로그인 하지 않은 세션 대상 Disconnect
	// 직렬화 패킷 생성 << 메시지
	// 채팅 보내기 (주변섹터)
	return false;
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_HEARTBEATE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket){
	// 로그인 여부 체크 -> 로그인 하지 않은 세션 대상 Disconnect
	// 마지막 통신시간 최신화
	return false;
}

void ChattingServer::Disconnect_Player(Player* p_player) {

}

void ChattingServer::OnError(int errorcode) {
}
