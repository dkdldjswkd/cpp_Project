#include "ChattingServer_Multi.h"
#include "CommonProtocol.h"
#include "../NetworkLib/Logger.h"
using namespace J_LIB;
using namespace std;

#define MAX_MSG 300

ChattingServer_Multi::ChattingServer_Multi(const char* systemFile, const char* server) : NetServer(systemFile, server) {
}

ChattingServer_Multi::~ChattingServer_Multi(){
}

// Session Connect ��
bool ChattingServer_Multi::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}
 
// Session �Ҵ� �� ȣ�� (Accept ������)
void ChattingServer_Multi::OnClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc(); // �ٸ� �����忡�� ��밡�ɼ� x
	p_player->Set(session_id);

	playerMap_lock.Lock_Exclusive();	
	player_map.insert({ session_id, p_player });
	playerMap_lock.Unlock_Exclusive();
}

// Session Release �� ȣ�� (Worker)
void ChattingServer_Multi::OnClientLeave(SESSION_ID session_id) {
	// Player map ����
	playerMap_lock.Lock_Exclusive();
	auto iter = player_map.find(session_id);
	Player* p_player = iter->second;
	player_map.erase(iter);
	playerMap_lock.Unlock_Exclusive();

	p_player->Reset(); // connect, login = false;

	// Secter set ����
	if (!p_player->sectorPos.CheckInvalid()) { // Sector ��Ͽ��� �Ǵ� (Sector ��Ŷ �ޱ������� ��� X)
		sector_lock[p_player->sectorPos.y][p_player->sectorPos.x].Lock_Exclusive();
		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player); 
		sector_lock[p_player->sectorPos.y][p_player->sectorPos.x].Unlock_Exclusive();
	}

	// Player ��ȯ
	playerPool.Free(p_player);
}

void ChattingServer_Multi::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	WORD type;
	try {
		*cs_contentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(session_id);
		return;
	}
	ProcPacket(session_id, type, cs_contentsPacket);
}

bool ChattingServer_Multi::ProcPacket(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket) {
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

bool ChattingServer_Multi::ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	// Player �˻�
	playerMap_lock.Lock_Shared();
	Player* p_player = player_map.find(session_id)->second;
	playerMap_lock.Unlock_Shared();

	INT64 accountNo;
	try {
		*cs_contentsPacket >> accountNo;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(session_id);
		return false;
	}

	// �α��� ����
	if (false == p_player->is_login) {
		try {
			cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
			cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
			Token token; // ������� ����
			cs_contentsPacket->Get_Data((char*)&token, 64);
			p_player->is_login = true;
		}
		catch (const PacketException& e) {
			LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
			Disconnect(session_id);
			return false;
		}
	}

	// �α��� ���� ��Ŷ ȸ��
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
	*p_packet << (BYTE)p_player->is_login;
	*p_packet << (INT64)accountNo;
	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);

	return p_player->is_login;
}

bool ChattingServer_Multi::ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	// Player �˻�
	playerMap_lock.Lock_Shared();
	Player* p_player = player_map.find(session_id)->second;
	playerMap_lock.Unlock_Shared();

	INT64 accountNo;
	WORD cur_x;
	WORD cur_y;

	try {
		*cs_contentsPacket >> accountNo;
		*cs_contentsPacket >> cur_x;
		*cs_contentsPacket >> cur_y;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(session_id);
		return false;
	}

	Sector prev_sector = p_player->sectorPos;
	Sector cur_sector = { cur_x, cur_y };
	p_player->Set_Sector(cur_sector);

	// Sector ���
	if (prev_sector.CheckInvalid()) {
		sector_lock[cur_y][cur_x].Lock_Exclusive();
		sectors_set[cur_y][cur_x].insert(p_player);
		sector_lock[cur_y][cur_x].Unlock_Exclusive();
	}
	// Sector �̵�
	else if (prev_sector != p_player->sectorPos) {
		// LOCK ���⼺ : LOCK �ּ� ũ��
		if (&sector_lock[prev_sector.y][prev_sector.x] < &sector_lock[cur_y][cur_x]) {
			sector_lock[prev_sector.y][prev_sector.x].Lock_Exclusive();
			sector_lock[cur_y][cur_x].Lock_Exclusive();
		}
		else {
			sector_lock[cur_y][cur_x].Lock_Exclusive();
			sector_lock[prev_sector.y][prev_sector.x].Lock_Exclusive();
		}
		// Player Sector �̵�
		sectors_set[prev_sector.y][prev_sector.x].erase(p_player);
		sectors_set[cur_y][cur_x].insert(p_player);
		sector_lock[prev_sector.y][prev_sector.x].Unlock_Exclusive();
		sector_lock[cur_y][cur_x].Unlock_Exclusive();
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

bool ChattingServer_Multi::ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	// Player �˻�
	playerMap_lock.Lock_Shared();
	Player* p_player = player_map.find(session_id)->second;
	playerMap_lock.Unlock_Shared();

	// >>
	INT64	accountNo;
	WORD	msgLen;
	WCHAR	msg[MAX_MSG]; // null ������
	try {
		*cs_contentsPacket >> accountNo;
		*cs_contentsPacket >> msgLen;
		cs_contentsPacket->Get_Data((char*)msg, msgLen);
		msg[msgLen / 2] = 0;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(session_id);
		return false;
	}

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

void ChattingServer_Multi::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	// 9���� Lock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		sector_lock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x].Lock_Shared();
	}
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		SendSector(send_packet, p_player->sectorAround.around[i]);
	}
	// 9���� Unlock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		sector_lock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x].Unlock_Shared();
	}
}

void ChattingServer_Multi::SendSector(PacketBuffer* send_packet, Sector sector) {
	for (auto iter = sectors_set[sector.y][sector.x].begin(); iter != sectors_set[sector.y][sector.x].end(); iter++) {
		Player* p_player = *iter;
		if (p_player->is_login) {
			SendPacket(p_player->session_id, send_packet);
		}
	}
}

void ChattingServer_Multi::Disconnect_Player(Player* p_player) {
}

void ChattingServer_Multi::OnError(int errorcode) {
}