#include "ChattingServerMulti.h"
#include "CommonProtocol.h"
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

	//AcquireSRWLockExclusive(&p_player->lock);
	p_player->Set_Connect(session_id);
	//ReleaseSRWLockExclusive(&p_player->lock);

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

	//AcquireSRWLockExclusive(&p_player->lock);
	p_player->Reset(); // connect, login = false;

	// Secter set 삭제
	if (p_player->sectorPos.CheckInvalid()) { // Sector 등록여부 판단 (Sector 패킷 받기전까지 등록 X)
		//ReleaseSRWLockExclusive(&p_player->lock);

		AcquireSRWLockExclusive(&sector_lock[p_player->sectorPos.y][p_player->sectorPos.x]); 
		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
		ReleaseSRWLockExclusive(&sector_lock[p_player->sectorPos.y][p_player->sectorPos.x]);
	}
	//else {
	//	ReleaseSRWLockExclusive(&p_player->lock);
	//}

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
		case en_PACKET_CS_CHAT_REQ_LOGIN:
			return ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(session_id, cs_contentsPacket);

		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
			return ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(session_id, cs_contentsPacket);

		case en_PACKET_CS_CHAT_REQ_MESSAGE:
			return ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(session_id, cs_contentsPacket);

		// ...
	}
}

bool ChattingServerMulti::ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	// Player 검색
	AcquireSRWLockShared(&playerMap_lock);
	Player* p_player = player_map.find(session_id)->second;
	ReleaseSRWLockShared(&playerMap_lock);

	INT64 accountNo;
	*cs_contentsPacket >> accountNo;

	AcquireSRWLockExclusive(&p_player->lock);
	// Set Login (login flag, id, nickname ...)
	ReleaseSRWLockExclusive(&p_player->lock);

	// 로그인 응답 패킷 Alloc
	// SendPacket();
	// 패킷 Free
}

bool ChattingServerMulti::ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket) {
	// Player 검색
	AcquireSRWLockShared(&playerMap_lock);
	Player* p_player = player_map.find(session_id)->second;
	ReleaseSRWLockShared(&playerMap_lock);

	INT64 accountNo;
	WORD sectorX;
	WORD sectorY;
	*cs_contentsPacket >> accountNo;
	*cs_contentsPacket >> sectorX;
	*cs_contentsPacket >> sectorY;

	//AcquireSRWLockExclusive(&p_player->lock);
	Sector cpy_sector = p_player->sectorPos;
	p_player->Set_Sector({ sectorX , sectorY });
	//ReleaseSRWLockExclusive(&p_player->lock);

	// 섹터 이동 방향에 따라 Sector AcquireSRWLockExclusive (섹터 잠금 순서 : ↗ ↑ → )
	// 순환대기 조건에 포함되지 않으므로 데드락 X
	// ex. (1,1) -> (0,1) // 왼쪽 섹터로 이동 시
	AcquireSRWLockExclusive(&sector_lock[sectorY][sectorX]); // (0,1)
	AcquireSRWLockExclusive(&sector_lock[cpy_sector.y][cpy_sector.x]); // (1,1)
	sectors_set[cpy_sector.y][cpy_sector.x].erase(p_player);
	sectors_set[sectorY][sectorX].insert(p_player);
	ReleaseSRWLockExclusive(&sector_lock[cpy_sector.y][cpy_sector.x]); // (1,1)
	ReleaseSRWLockExclusive(&sector_lock[sectorY][sectorX]); // (0,1)

	// 섹터 이동 응답 패킷 Alloc
	// SendPacket();
	// 패킷 Free
}

bool ChattingServerMulti::ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket){
	// Player 검색
	AcquireSRWLockShared(&playerMap_lock);
	Player* p_player = player_map.find(session_id)->second;
	ReleaseSRWLockShared(&playerMap_lock);

	// 섹터 이동 응답 패킷 Alloc
	AcquireSRWLockShared(&p_player->lock);
	// 패킷 Set
	ReleaseSRWLockShared(&p_player->lock);
	//SendSectorAround();
	// 패킷 Free
}

void ChattingServerMulti::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	// (최대) 9방향 AcquireSRWLockShared (유효한 섹터만, ex. (0,0)일경우 4섹터)
	// (최대) 9방향 SendSector
	// (최대) 9방향 ReleaseSRWLockShared
}

void ChattingServerMulti::SendSector(PacketBuffer* send_packet, Sector sector) {
	// 밖에서 해당 Sector Lock
	auto iter = sectors_set[sector.y][sector.x].begin();
	for (; iter != sectors_set[sector.y][sector.x].end(); iter++) {
		Player* p_player = *iter; // 재사용 X

		// AcquireSRWLockShared(&p_player->lock);
		if (p_player->is_connect) {
			SendPacket(p_player->session_id, send_packet);
		}
		// AcquireSRWLockShared(&p_player->lock);
	}
}

void ChattingServerMulti::Disconnect_Player(Player* p_player) {
}

void ChattingServerMulti::OnError(int errorcode) {
}