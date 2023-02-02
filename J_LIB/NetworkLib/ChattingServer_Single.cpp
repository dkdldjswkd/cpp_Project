#include "ChattingServer_Single.h"
#include "CommonProtocol.h"
#include "Logger.h"
using namespace J_LIB;
using namespace std;

#define MAX_MSG 300

ChattingServer_Single::ChattingServer_Single() {
	updateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	updateThread = thread([this]{UpdateFunc(); });
}

ChattingServer_Single::~ChattingServer_Single(){
}

// Session Connect 시
bool ChattingServer_Single::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}
 
// Session 할당 후
void ChattingServer_Single::OnClientJoin(SESSION_ID session_id) {
	JobQueuing(session_id, JOB_TYPE_CLIENT_JOIN);
}

void ChattingServer_Single::OnClientLeave(SESSION_ID session_id) {
	JobQueuing(session_id, JOB_TYPE_CLIENT_LEAVE);
}

// Player 객체 생성 (로그인 상태 X)
void ChattingServer_Single::ProcJob_ClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc();
	p_player->Set_Connect(session_id);
	player_map.insert({ session_id, p_player });
}

void ChattingServer_Single::ProcJob_ClientLeave(SESSION_ID session_id) {
	// Player 검색
	auto iter = player_map.find(session_id);
	Player* p_player = iter->second;

	// 컨테이너에서 삭제
	player_map.erase(iter);

	// Sector 에서 삭제
	if (!p_player->sectorPos.Is_Invalid()) {
		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
	}

	// Player 반환
	p_player->Reset();
	playerPool.Free(p_player);
}

void ChattingServer_Single::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	WORD type;
	try {
		*cs_contentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "impossible : >> type");
		Disconnect(session_id);
		return;
	}

	// INVALID Packet type
	if ( (5 < type) || (type == 2) || (type == 4) ) { // 1, 3, 5
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "OnRecv() : INVALID Packet type (%d)", type);
		Disconnect(session_id);
		return;
	}

	cs_contentsPacket->Increment_refCount();
	JobQueuing(session_id, type, cs_contentsPacket);
}

void ChattingServer_Single::UpdateFunc() {
	Job* p_job;
	for (;;) {
		WaitForSingleObject(updateEvent, INFINITE);
		while (jobQ.Dequeue(&p_job)) {
			updateTPS++;

			ProcJob(p_job->session_id, p_job->type, p_job->p_packet);
			jobPool.Free(p_job);
		}
	}
}

void ChattingServer_Single::ProcJob(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket) {
	switch (type) {
		// 패킷처리
		case en_PACKET_CS_CHAT_REQ_LOGIN:
			ProcJob_en_PACKET_CS_CHAT_REQ_LOGIN(session_id, cs_contentsPacket);
			PacketBuffer::Free(cs_contentsPacket);
			return;

		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
			ProcJob_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(session_id, cs_contentsPacket);
			PacketBuffer::Free(cs_contentsPacket);
			return;

		case en_PACKET_CS_CHAT_REQ_MESSAGE:
			ProcJob_en_PACKET_CS_CHAT_REQ_MESSAGE(session_id, cs_contentsPacket);
			PacketBuffer::Free(cs_contentsPacket);
			return;

		// OnFunc 처리
		case JOB_TYPE_CLIENT_JOIN:
			return ProcJob_ClientJoin(session_id);

		case JOB_TYPE_CLIENT_LEAVE:
			return ProcJob_ClientLeave(session_id);

		default: {
			LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "ProcJob INVALID Packet type : %d", type);
			Disconnect(session_id);
			break;
		}
	}
}

bool ChattingServer_Single::ProcJob_en_PACKET_CS_CHAT_REQ_LOGIN(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	INT64 accountNo;

	auto iter = player_map.find(session_id);
	if (player_map.end() == iter) CRASH();
	Player* p_player = iter->second;
	//Player* p_player = player_map.find(session_id)->second;

	// 이미 로그인 된 플레이어
	if (p_player->is_login) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Already login!!");
		Disconnect(session_id);
		return false;
	}
	else {
		p_player->Set_Login();
		try {
			*cs_contentsPacket >> accountNo;
			cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
			cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
			cs_contentsPacket->Get_Data((char*)p_player->sessionKey, 64); // 인증키 확인해야함
		}
		catch (const PacketException& e) {
			LOG("ChattingServer-Single", LOG_LEVEL_WARN, "impossible : >> Login packet");
			Disconnect(session_id);
			return false;
		}
	}

	// 로그인 응답 패킷 회신
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
	*p_packet << (BYTE)p_player->is_login;
	*p_packet << (INT64)accountNo;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);

	return p_player->is_login;
}

bool ChattingServer_Single::ProcJob_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	auto iter = player_map.find(session_id);
	if (player_map.end() == iter) CRASH();
	Player* p_player = iter->second;
	//Player* p_player = player_map.find(session_id)->second;

	// 로그인 상태가 아닌 플레이어
	if (!p_player->is_login) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "is not login!!");
		Disconnect(session_id);
		return false;
	}

	INT64 accountNo;
	Sector cur_sector;
	try {
		*cs_contentsPacket >> accountNo;
		*cs_contentsPacket >> cur_sector.x;
		*cs_contentsPacket >> cur_sector.y;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "impossible : >> sector move packet");
		Disconnect(session_id);
		return false;
	}

	// 유효하지 않은 섹터로 이동 시도
	if (cur_sector.Is_Invalid()) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "sector move Packet : Invalid Sector!!");
		Disconnect(session_id);
		return false;
	}

	// 이 전에 위치하던 Sector erase
	if (!p_player->sectorPos.Is_Invalid()) {
		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
	}
	p_player->Set_Sector(cur_sector); 
	sectors_set[cur_sector.y][cur_sector.x].insert(p_player);

	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	*p_packet << (INT64)accountNo;
	*p_packet << (WORD)cur_sector.x;
	*p_packet << (WORD)cur_sector.y;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);
	return true;
}

bool ChattingServer_Single::ProcJob_en_PACKET_CS_CHAT_REQ_MESSAGE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	auto iter = player_map.find(session_id);
	if (player_map.end() == iter) CRASH();
	Player* p_player = iter->second;
	//Player* p_player = player_map.find(session_id)->second;

	// 로그인 상태가 아닌 플레이어
	if (!p_player->is_login) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "is not login!!");
		Disconnect(session_id);
		return false;
	}

	// >>
	INT64	accountNo;
	WORD	msgLen;
	WCHAR	msg[MAX_MSG]; // null 미포함
	try {
		*cs_contentsPacket >> accountNo;
		*cs_contentsPacket >> msgLen;
		cs_contentsPacket->Get_Data((char*)msg, msgLen);
	}
	catch (const PacketException& e) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "impossible : >> catting packet");
		Disconnect(session_id);
		return false;
	}
	msg[msgLen / 2] = 0;

	// <<
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE;
	*p_packet << (INT64)accountNo;
	p_packet->Put_Data((char*)p_player->id, ID_LEN * 2);
	p_packet->Put_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
	*p_packet << (WORD)msgLen;
	p_packet->Put_Data((char*)msg, msgLen);

	SendSectorAround(p_player, p_packet);
	PacketBuffer::Free(p_packet);
	return true;
}

void ChattingServer_Single::JobQueuing(SESSION_ID session_id, WORD type, PacketBuffer* p_packet) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(session_id, type, p_packet);
	jobQ.Enqueue(p_job);
	SetEvent(updateEvent);
}

void ChattingServer_Single::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		SendSector(send_packet, p_player->sectorAround.around[i]);
	}
}

void ChattingServer_Single::SendSector(PacketBuffer* send_packet, Sector sector) {
	auto iter = sectors_set[sector.y][sector.x].begin();
	for (; iter != sectors_set[sector.y][sector.x].end(); iter++) {
		SendPacket((*iter)->session_id, send_packet);
	}
}

void ChattingServer_Single::OnError(int errorcode) {
}