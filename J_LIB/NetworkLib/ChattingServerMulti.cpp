#include "ChattingServerMulti.h"
#include "CommonProtocol.h"
#include "Logger.h"
using namespace J_LIB;
using namespace std;

#define MAX_MSG 300

ChattingServerMulti::ChattingServerMulti() {
	InitializeSRWLock(&playerMap_lock);
	for (int i = 0; i < SECTOR_MAX_Y; i++) {
		for (int j = 0; j < SECTOR_MAX_X; j++) {
			InitializeSRWLock(&sector_lock[i][j]);
		}
	}
}

ChattingServerMulti::~ChattingServerMulti(){
}

// Session Connect 시
bool ChattingServerMulti::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}
 
// Session 할당 후 호출 (Accept 스레드)
void ChattingServerMulti::OnClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc(); // 다른 스레드에서 사용가능성 x

	p_player->Set_Connect(session_id);

	AcquireSRWLockExclusive(&playerMap_lock);
	player_map.insert({ session_id, p_player });
	ReleaseSRWLockExclusive(&playerMap_lock);
}

// Session Release 후 호출 (Worker)
void ChattingServerMulti::OnClientLeave(SESSION_ID session_id) {
	// Player map 삭제
	AcquireSRWLockExclusive(&playerMap_lock);
	auto iter = player_map.find(session_id);
	Player* p_player = iter->second;
	player_map.erase(iter);
	ReleaseSRWLockExclusive(&playerMap_lock);

	p_player->Reset(); // connect, login = false;

	// Secter set 삭제
	if (!p_player->sectorPos.CheckInvalid()) { // Sector 등록여부 판단 (Sector 패킷 받기전까지 등록 X)
		AcquireSRWLockExclusive(&sector_lock[p_player->sectorPos.y][p_player->sectorPos.x]);
		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player); 
		ReleaseSRWLockExclusive(&sector_lock[p_player->sectorPos.y][p_player->sectorPos.x]);
	}

	// Player 반환
	playerPool.Free(p_player);
}

void ChattingServerMulti::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	WORD type;
	*cs_contentsPacket >> type;
	ProcPacket(session_id, type, cs_contentsPacket);
}

bool ChattingServerMulti::ProcPacket(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket) {
	switch (type) {
		case en_PACKET_CS_CHAT_REQ_LOGIN: {
			return ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(session_id, cs_contentsPacket);
		}
		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE: {
			return ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(session_id, cs_contentsPacket);
		}
		case en_PACKET_CS_CHAT_REQ_MESSAGE: {
			return ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(session_id, cs_contentsPacket);
		}
		default: {
			LOG("ChattingServer-Multi", LOG_LEVEL_WARN, "INVALID Packet type : %d", type);
			Disconnect(session_id);
			break;
		}
	}
}

bool ChattingServerMulti::ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	// Player 검색
	AcquireSRWLockShared(&playerMap_lock);
	Player* p_player = player_map.find(session_id)->second;
	ReleaseSRWLockShared(&playerMap_lock);

	INT64 accountNo;
	*cs_contentsPacket >> accountNo;

	if (false == p_player->is_login) {
		// 로그인 성공
		p_player->Set_Login();
		cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
		cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
		cs_contentsPacket->Get_Data((char*)p_player->sessionKey, 64); // 인증키 확인해야함
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

bool ChattingServerMulti::ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	// Player 검색
	AcquireSRWLockShared(&playerMap_lock);
	Player* p_player = player_map.find(session_id)->second;
	ReleaseSRWLockShared(&playerMap_lock);

	INT64 accountNo;
	WORD cur_x;
	WORD cur_y;
	*cs_contentsPacket >> accountNo;
	*cs_contentsPacket >> cur_x;
	*cs_contentsPacket >> cur_y;

	Sector prev_sector = p_player->sectorPos;
	Sector cur_sector = { cur_x, cur_y };
	p_player->Set_Sector(cur_sector);

	// Sector 등록
	if (prev_sector.CheckInvalid()) {
		AcquireSRWLockExclusive(&sector_lock[cur_y][cur_x]);
		sectors_set[cur_y][cur_x].insert(p_player);
		ReleaseSRWLockExclusive(&sector_lock[cur_y][cur_x]);
	}
	// Sector 이동
	else if (prev_sector != p_player->sectorPos) {
		// LOCK 방향성 : LOCK 주소 크기
		if (&sector_lock[prev_sector.y][prev_sector.x] < &sector_lock[cur_y][cur_x]) {
			AcquireSRWLockExclusive(&sector_lock[prev_sector.y][prev_sector.x]);
			AcquireSRWLockExclusive(&sector_lock[cur_y][cur_x]);
		}
		else {
			AcquireSRWLockExclusive(&sector_lock[cur_y][cur_x]);
			AcquireSRWLockExclusive(&sector_lock[prev_sector.y][prev_sector.x]);
		}
		// Player Sector 이동
		sectors_set[prev_sector.y][prev_sector.x].erase(p_player);
		sectors_set[cur_y][cur_x].insert(p_player);
		ReleaseSRWLockExclusive(&sector_lock[prev_sector.y][prev_sector.x]);
		ReleaseSRWLockExclusive(&sector_lock[cur_y][cur_x]);
	}

	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	*p_packet << (INT64)accountNo;
	*p_packet << (WORD)cur_x;
	*p_packet << (WORD)cur_y;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);
	return true;
}

bool ChattingServerMulti::ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	// Player 검색
	AcquireSRWLockShared(&playerMap_lock);
	Player* p_player = player_map.find(session_id)->second;
	ReleaseSRWLockShared(&playerMap_lock);

	// >>
	INT64	accountNo;
	WORD	msgLen;
	WCHAR	msg[MAX_MSG]; // null 미포함
	*cs_contentsPacket >> accountNo;
	*cs_contentsPacket >> msgLen;
	cs_contentsPacket->Get_Data((char*)msg, msgLen);
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

void ChattingServerMulti::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	// 9방향 Lock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		AcquireSRWLockShared(&sector_lock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x]);
	}
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		SendSector(send_packet, p_player->sectorAround.around[i]);
	}
	// 9방향 Unlock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		ReleaseSRWLockShared(&sector_lock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x]);
	}
}

void ChattingServerMulti::SendSector(PacketBuffer* send_packet, Sector sector) {
	for (auto iter = sectors_set[sector.y][sector.x].begin(); iter != sectors_set[sector.y][sector.x].end(); iter++) {
		Player* p_player = *iter;
		if (p_player->is_connect) {
			SendPacket(p_player->session_id, send_packet);
		}
	}
}

void ChattingServerMulti::Disconnect_Player(Player* p_player) {
}

void ChattingServerMulti::OnError(int errorcode) {
}