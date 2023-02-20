#include "ChattingServer_Single.h"
#include "CommonProtocol.h"
#include "../NetworkLib/Logger.h"
using namespace J_LIB;
using namespace std;

#define MAX_MSG 300

ChattingServer_Single::ChattingServer_Single(const char* systemFile, const char* server) : NetworkLib(systemFile, server) {
	updateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	tokenEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	updateThread = thread([this] {UpdateFunc(); });
	tokenThread = thread([this] {TokenAuthFunc(); });
	connectorRedis.connect();
}

ChattingServer_Single::~ChattingServer_Single(){
}

//////////////////////////////
// JOB QUEUING
//////////////////////////////

void ChattingServer_Single::OnClientJoin(SESSION_ID session_id) {
	JobQueuing(session_id, JOB_TYPE_CLIENT_JOIN);
}

void ChattingServer_Single::OnClientLeave(SESSION_ID session_id) {
	JobQueuing(session_id, JOB_TYPE_CLIENT_LEAVE);
}

void ChattingServer_Single::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	WORD type;
	try {
		*cs_contentsPacket >> type;
	}
	catch (const PacketException& e) {
		//LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> type");
		Disconnect(session_id);
		return;
	}

	// INVALID Packet type
	if ((5 < type) || (type == 2) || (type == 4)) { // 1, 3, 5
		//LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // OnRecv() : INVALID Packet type (%d)", type);
		Disconnect(session_id);
		return;
	}

	cs_contentsPacket->Increment_refCount();
	JobQueuing(session_id, type, cs_contentsPacket);
}

//////////////////////////////
// JOB PROC
//////////////////////////////

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
			ProcJob_ClientJoin(session_id);
			return;

		case JOB_TYPE_CLIENT_LEAVE:
			ProcJob_ClientLeave(session_id);
			return;

		case JOB_TYPE_CLIENT_LOGIN_SUCCESS:
			ProcJob_ClientLoginSuccess(session_id);
			return;

		case JOB_TYPE_CLIENT_LOGIN_FAIL:
			ProcJob_ClientLoginFail(session_id);
			return;

		default:
			LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "Disconnect // ProcJob INVALID Packet type : %d", type);
			Disconnect(session_id);
			return;
	}
}

// Player 객체 생성 (로그인 상태 X)
void ChattingServer_Single::ProcJob_ClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc();
	p_player->Set(session_id);
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

void ChattingServer_Single::ProcJob_en_PACKET_CS_CHAT_REQ_LOGIN(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	auto iter = player_map.find(session_id);
	if (iter == player_map.end()) {
		LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "REQ_LOGIN() : player_map.find(session_id) == player_map.end()");
		return;
	}
	Player* p_player = iter->second;

	// 이미 로그인 된 플레이어
	if (p_player->is_login) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // Already login!!");
		Disconnect(session_id);
		return;
	}

#ifdef ON_LOGIN
	AccountToken* p_at = tokenPool.Alloc();
	p_at->sessionID = session_id;
	try {
		cs_contentsPacket->Get_Data((char*)&p_at->accountNo, sizeof(ACCOUNT_NO));
		cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
		cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
		cs_contentsPacket->Get_Data((char*)&p_at->token, 64);
}
	catch (const PacketException& e) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> Login packet");
		tokenPool.Free(p_at);
		Disconnect(session_id);
		return;
	}
	p_player->accountNo = p_at->accountNo;
	tokenQ.Enqueue(p_at);
	SetEvent(tokenEvent);
#endif
#ifndef ON_LOGIN
	p_player->Set(session_id);
	try {
		*cs_contentsPacket >> p_player->accountNo;
		cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
		cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
		Token token; // 사용하지 않음
		cs_contentsPacket->Get_Data((char*)&token, 64);
	}
	catch (const PacketException& e) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> Login packet");
		Disconnect(session_id);
		return;
	}

	// 로그인 성공
	p_player->is_login = true;

	// 로그인 응답 패킷 회신
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
	*p_packet << (BYTE)p_player->is_login;
	*p_packet << (INT64)p_player->accountNo;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);
#endif
}

// JOB_TYPE_CLIENT_LOGIN_SUCCESS
void ChattingServer_Single::ProcJob_ClientLoginSuccess(SESSION_ID session_id) {
	auto iter = player_map.find(session_id);
	if (iter == player_map.end()) { // 로그인 결과 패킷 회신 전 연결끊킴
		return;
	}
	Player* p_player = iter->second;

	// 로그인 성공
	p_player->is_login = true;

	// 로그인 응답 패킷(성공) 회신
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
	*p_packet << (BYTE)p_player->is_login;
	*p_packet << (INT64)p_player->accountNo;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);
}

// JOB_TYPE_CLIENT_LOGIN_FAIL
int failCount = 0;
void ChattingServer_Single::ProcJob_ClientLoginFail(SESSION_ID session_id) {
	auto iter = player_map.find(session_id);
	if (iter == player_map.end()) {  // 로그인 결과 패킷 회신 전 연결끊킴
		return;
	}
	Player* p_player = iter->second;

	// 로그인 응답 패킷(실패) 회신
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
	*p_packet << (BYTE)false;
	*p_packet << (INT64)p_player->accountNo;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);

	InterlockedIncrement((LONG*)&failCount);
}

void ChattingServer_Single::ProcJob_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	auto iter = player_map.find(session_id);
	if (iter == player_map.end()) {
		LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "REQ_SECTOR_MOVE() : player_map.find(session_id) == player_map.end()");
		return;
	}
	Player* p_player = iter->second;

	// 로그인 상태가 아닌 플레이어
	if (!p_player->is_login) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // is not login!!");
		Disconnect(session_id);
		return ;
	}

	INT64 accountNo;
	Sector cur_sector;
	try {
		*cs_contentsPacket >> accountNo;
		*cs_contentsPacket >> cur_sector.x;
		*cs_contentsPacket >> cur_sector.y;
	}
	catch (const PacketException& e) {
		//LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> sector move packet");
		Disconnect(session_id);
		return ;
	}

	// 유효하지 않은 섹터로 이동 시도
	if (cur_sector.Is_Invalid()) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // sector move Packet : Invalid Sector!!");
		Disconnect(session_id);
		return ;
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
}

bool ChattingServer_Single::ProcJob_en_PACKET_CS_CHAT_REQ_MESSAGE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	Player* p_player = player_map.find(session_id)->second;
	if (nullptr == p_player)
		return false;

	// 로그인 상태가 아닌 플레이어
	if (!p_player->is_login) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // is not login!!");
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
		//LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> chatting packet");
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

//////////////////////////////
// UPDATE THREAD
//////////////////////////////

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

void ChattingServer_Single::TokenAuthFunc() {
	Token redisToken;
	AccountToken* p_at;

	for (;;) {
		WaitForSingleObject(tokenEvent, INFINITE);
		while (tokenQ.Dequeue(&p_at)) {
			char chattingKey[100];
			snprintf(chattingKey, 100, "%d.chatting", p_at->accountNo);

			std::future<cpp_redis::reply> future_reply = connectorRedis.get(chattingKey);
			connectorRedis.sync_commit();
			cpp_redis::reply reply = future_reply.get();
			if (reply.is_string()) {
				#pragma warning(suppress : 4996)
				strncpy((char*)&redisToken, reply.as_string().c_str(), 64);
			}
			else {
				ZeroMemory(&redisToken, sizeof(redisToken));
			}

			// 유효 세션
			if (0 == strncmp(redisToken.buf, p_at->token.buf, 64)) {
				connectorRedis.del({ chattingKey });
				connectorRedis.sync_commit();
				JobQueuing(p_at->sessionID, JOB_TYPE_CLIENT_LOGIN_SUCCESS);
			}
			// 유효하지 않은 세션
			else {
				JobQueuing(p_at->sessionID, JOB_TYPE_CLIENT_LOGIN_FAIL);
				LOG("ChattingServer-Single", LOG_LEVEL_WARN, "INVALID TOKEN AccountNo(%d)	Redis(%.*s), User(%.*s)", p_at->accountNo, 64, redisToken.buf, 64, p_at->token.buf);
			}
			tokenPool.Free(p_at);
		}
	}
}

//////////////////////////////
// OTHER
//////////////////////////////

void ChattingServer_Single::JobQueuing(SESSION_ID session_id, WORD type, PacketBuffer* p_packet) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(session_id, type, p_packet);
	jobQ.Enqueue(p_job);
	SetEvent(updateEvent);
}

void ChattingServer_Single::JobQueuing(SESSION_ID session_id, WORD type) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(session_id, type);
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

// Session Connect 시
bool ChattingServer_Single::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}