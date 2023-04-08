#include "ChatServerMT.h"
#include "CommonProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include "ChatServerMT.h"
using namespace std;

#define MAX_MSG 300

ChatServerMT::ChatServerMT(const char* systemFile, const char* server) : NetServer(systemFile, server) {
}

ChatServerMT::~ChatServerMT(){
}

// Session Connect ��
bool ChatServerMT::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}
 
// Session �Ҵ� �� ȣ�� (Accept ������)
void ChatServerMT::OnClientJoin(SessionId sessionId) {
	Player* p_player = playerPool.Alloc(); // �ٸ� �����忡�� ��밡�ɼ� x
	p_player->Set(sessionId);

	playerMapLock.Lock_Exclusive();	
	playerMap.insert({ sessionId, p_player });
	playerMapLock.Unlock_Exclusive();
}

// Session Release �� ȣ�� (Worker)
void ChatServerMT::OnClientLeave(SessionId sessionId) {
	// Player map ����
	playerMapLock.Lock_Exclusive();
	auto iter = playerMap.find(sessionId);
	Player* p_player = iter->second;
	playerMap.erase(iter);
	playerMapLock.Unlock_Exclusive();

	p_player->Reset(); // connect, login = false;

	// Secter set ����
	if (!p_player->sectorPos.IsInvalid()) { // Sector ��Ͽ��� �Ǵ� (Sector ��Ŷ �ޱ������� ��� X)
		sectorLock[p_player->sectorPos.y][p_player->sectorPos.x].Lock_Exclusive();
		sectorSet[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player); 
		sectorLock[p_player->sectorPos.y][p_player->sectorPos.x].Unlock_Exclusive();
	}

	// Player ��ȯ
	playerPool.Free(p_player);
}

void ChatServerMT::OnRecv(SessionId sessionId, PacketBuffer* csContentsPacket) {
	WORD type;
	try {
		*csContentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(sessionId);
		return;
	}
	ProcPacket(sessionId, type, csContentsPacket);
}

bool ChatServerMT::ProcPacket(SessionId sessionId, WORD type, PacketBuffer* csContentsPacket) {
	switch (type) {
		case en_PACKET_CS_CHAT_REQ_LOGIN: {
			return ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(sessionId, csContentsPacket);
		}
		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE: {
			return ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(sessionId, csContentsPacket);
		}
		case en_PACKET_CS_CHAT_REQ_MESSAGE: {
			return ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(sessionId, csContentsPacket);
		}
		default: {
			LOG("ChattingServer-Multi", LOG_LEVEL_WARN, "INVALID Packet type : %d", type);
			Disconnect(sessionId);
			break;
		}
	}
}

bool ChatServerMT::ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(SessionId sessionId, PacketBuffer* csContentsPacket){
	// Player �˻�
	playerMapLock.Lock_Shared();
	Player* p_player = playerMap.find(sessionId)->second;
	playerMapLock.Unlock_Shared();

	INT64 accountNo;
	try {
		*csContentsPacket >> accountNo;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(sessionId);
		return false;
	}

	// �α��� ����
	if (false == p_player->isLogin) {
		try {
			csContentsPacket->GetData((char*)p_player->id, ID_LEN * 2);
			csContentsPacket->GetData((char*)p_player->nickname, NICKNAME_LEN * 2);
			Token token; // ������� ����
			csContentsPacket->GetData((char*)&token, 64);
			p_player->isLogin = true;
		}
		catch (const PacketException& e) {
			LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
			Disconnect(sessionId);
			return false;
		}
	}

	// �α��� ���� ��Ŷ ȸ��
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
	*p_packet << (BYTE)p_player->isLogin;
	*p_packet << (INT64)accountNo;
	SendPacket(sessionId, p_packet);
	PacketBuffer::Free(p_packet);

	return p_player->isLogin;
}

bool ChatServerMT::ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SessionId sessionId, PacketBuffer* csContentsPacket) {
	// Player �˻�
	playerMapLock.Lock_Shared();
	Player* p_player = playerMap.find(sessionId)->second;
	playerMapLock.Unlock_Shared();

	INT64 accountNo;
	WORD cur_x;
	WORD cur_y;

	try {
		*csContentsPacket >> accountNo;
		*csContentsPacket >> cur_x;
		*csContentsPacket >> cur_y;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(sessionId);
		return false;
	}

	Sector prev_sector = p_player->sectorPos;
	Sector cur_sector = { cur_x, cur_y };
	p_player->SetSector(cur_sector);

	// Sector ���
	if (prev_sector.IsInvalid()) {
		sectorLock[cur_y][cur_x].Lock_Exclusive();
		sectorSet[cur_y][cur_x].insert(p_player);
		sectorLock[cur_y][cur_x].Unlock_Exclusive();
	}
	// Sector �̵�
	else if (prev_sector != p_player->sectorPos) {
		// LOCK ���⼺ : LOCK �ּ� ũ��
		if (&sectorLock[prev_sector.y][prev_sector.x] < &sectorLock[cur_y][cur_x]) {
			sectorLock[prev_sector.y][prev_sector.x].Lock_Exclusive();
			sectorLock[cur_y][cur_x].Lock_Exclusive();
		}
		else {
			sectorLock[cur_y][cur_x].Lock_Exclusive();
			sectorLock[prev_sector.y][prev_sector.x].Lock_Exclusive();
		}
		// Player Sector �̵�
		sectorSet[prev_sector.y][prev_sector.x].erase(p_player);
		sectorSet[cur_y][cur_x].insert(p_player);
		sectorLock[prev_sector.y][prev_sector.x].Unlock_Exclusive();
		sectorLock[cur_y][cur_x].Unlock_Exclusive();
	}

	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	*p_packet << (INT64)accountNo;
	*p_packet << (WORD)cur_x;
	*p_packet << (WORD)cur_y;
	SendPacket(sessionId, p_packet);
	PacketBuffer::Free(p_packet);
	return true;
}

bool ChatServerMT::ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(SessionId sessionId, PacketBuffer* csContentsPacket){
	// Player �˻�
	playerMapLock.Lock_Shared();
	Player* p_player = playerMap.find(sessionId)->second;
	playerMapLock.Unlock_Shared();

	// >>
	INT64	accountNo;
	WORD	msgLen;
	WCHAR	msg[MAX_MSG]; // null ������
	try {
		*csContentsPacket >> accountNo;
		*csContentsPacket >> msgLen;
		csContentsPacket->GetData((char*)msg, msgLen);
		msg[msgLen / 2] = 0;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer_Multi", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(sessionId);
		return false;
	}

	// <<
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE;
	*p_packet << (INT64)accountNo;
	p_packet->PutData((char*)p_player->id, ID_LEN * 2);
	p_packet->PutData((char*)p_player->nickname, NICKNAME_LEN * 2);
	*p_packet << (WORD)msgLen;
	p_packet->PutData((char*)msg, msgLen);

	SendSectorAround(p_player, p_packet);
	PacketBuffer::Free(p_packet);
	return true;
}

void ChatServerMT::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	// 9���� Lock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		sectorLock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x].Lock_Shared();
	}
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		SendSector(send_packet, p_player->sectorAround.around[i]);
	}
	// 9���� Unlock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		sectorLock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x].Unlock_Shared();
	}
}

void ChatServerMT::SendSector(PacketBuffer* send_packet, Sector sector) {
	for (auto iter = sectorSet[sector.y][sector.x].begin(); iter != sectorSet[sector.y][sector.x].end(); iter++) {
		Player* p_player = *iter;

		if (p_player->isLogin) {
			SendPacket(p_player->sessionId, send_packet);
		}
	}
}

void ChatServerMT::OnServerStop(){
}