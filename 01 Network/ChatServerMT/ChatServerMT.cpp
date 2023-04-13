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

void ChatServerMT::OnServerStop() {
}

// Session Connect ��
bool ChatServerMT::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}
 
// Session �Ҵ� �� ȣ�� (Accept ������)
void ChatServerMT::OnClientJoin(SessionId sessionId) {
	Player* p_player = playerPool.Alloc();
	p_player->Set(sessionId);

	playerMapLock.Lock();	
	playerMap.insert({ sessionId, p_player });
	playerMapLock.Unlock();
}

// Session Release �� ȣ�� (Worker)
void ChatServerMT::OnClientLeave(SessionId sessionId) {
	// Player map ����
	playerMapLock.Lock();
	auto iter = playerMap.find(sessionId);
	Player* p_player = iter->second;
	playerMap.erase(iter);
	playerMapLock.Unlock();

	p_player->Reset(); // connect, login = false;

	// Secter set ����
	if (!p_player->sectorPos.IsInvalid()) { // Sector ��Ͽ��� �Ǵ� (Sector ��Ŷ �ޱ������� ��� X)
		sectorLock[p_player->sectorPos.y][p_player->sectorPos.x].Lock();
		sectorSet[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player); 
		sectorLock[p_player->sectorPos.y][p_player->sectorPos.x].Unlock();
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
		LOG("ChatServerMT", LOG_LEVEL_WARN, "Disconnect // impossible Packet Type >>");
		Disconnect(sessionId);
		return;
	}
	switch (type) {
		case en_PACKET_CS_CHAT_REQ_LOGIN: {
			INT64 accountNo;
			WCHAR id[20];
			WCHAR nickname[20];
			char  token[64];
			try {
				*csContentsPacket >> accountNo;
				csContentsPacket->GetData((char*)id, ID_LEN * 2);
				csContentsPacket->GetData((char*)nickname, NICKNAME_LEN * 2);
				csContentsPacket->GetData(token, 64);
			}
			catch (const PacketException& e) {
				LOG("ChatServerMT", LOG_LEVEL_WARN, "Disconnect // impossible : >>, Packet Type : REQ_LOGIN");
				Disconnect(sessionId);
				return;
			}

			// Player �˻�
			playerMapLock.SharedLock();
			Player* p_player = playerMap.find(sessionId)->second;
			playerMapLock.ReleaseSharedLock();

			// �ߺ� �α��� üũ
			if (p_player->isLogin) {
				LOG("ChatServerMT", LOG_LEVEL_WARN, "Disconnect // Duplicate logins");
				Disconnect(sessionId);
				return;
			}
			p_player->isLogin = true;

			// �α��� ���� set
			p_player->accountNo = accountNo;
			wcsncpy_s(p_player->id, ID_LEN, id, ID_LEN - 1);
			wcsncpy_s(p_player->nickname, NICKNAME_LEN, nickname, NICKNAME_LEN - 1);

			// �α��� ���� ��Ŷ ȸ��
			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
			*p_packet << (BYTE)p_player->isLogin;
			*p_packet << (INT64)accountNo;
			SendPacket(sessionId, p_packet);
			PacketBuffer::Free(p_packet);
			break;
		}
		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE: {
			INT64 accountNo;
			Sector curSector;
			try {
				*csContentsPacket >> accountNo;
				*csContentsPacket >> curSector.x;
				*csContentsPacket >> curSector.y;
			}
			catch (const PacketException& e) {
				LOG("ChatServerMT", LOG_LEVEL_WARN, "Disconnect // impossible : >>, Packet Type : SECTOR_MOVE");
				Disconnect(sessionId);
				return;
			}

			// ���� ��ȿ�� �˻�
			if (curSector.IsInvalid()) {
				LOG("ChatServerMT", LOG_LEVEL_WARN, "Disconnect // Invalid Sector");
				Disconnect(sessionId);
				break;
			}

			// Player �˻�
			playerMapLock.SharedLock();
			Player* p_player = playerMap.find(sessionId)->second;
			playerMapLock.ReleaseSharedLock();

			// Player Sector ������Ʈ (Set�� �ݿ� x, ���� Sector ���)
			Sector prevSector = p_player->sectorPos;
			p_player->SetSector(curSector);

			// Sector ���
			if (prevSector.IsInvalid()) {
				sectorLock[curSector.y][curSector.x].Lock();
				sectorSet[curSector.y][curSector.x].insert(p_player);
				sectorLock[curSector.y][curSector.x].Unlock();
			}
			// Sector �̵�
			else if (prevSector != p_player->sectorPos) {
				// LOCK ���⼺ : LOCK �ּ� ũ��
				if (&sectorLock[prevSector.y][prevSector.x] < &sectorLock[curSector.y][curSector.x]) {
					sectorLock[prevSector.y][prevSector.x].Lock();
					sectorLock[curSector.y][curSector.x].Lock();
				}
				else {
					sectorLock[curSector.y][curSector.x].Lock();
					sectorLock[prevSector.y][prevSector.x].Lock();
				}
				// Player Sector �̵�
				sectorSet[prevSector.y][prevSector.x].erase(p_player);
				sectorSet[curSector.y][curSector.x].insert(p_player);
				sectorLock[prevSector.y][prevSector.x].Unlock();
				sectorLock[curSector.y][curSector.x].Unlock();
			}

			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
			*p_packet << (INT64)accountNo;
			*p_packet << (WORD)curSector.x;
			*p_packet << (WORD)curSector.y;
			SendPacket(sessionId, p_packet);
			PacketBuffer::Free(p_packet);
			break;
		}
		case en_PACKET_CS_CHAT_REQ_MESSAGE: {
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
				LOG("ChatServerMT", LOG_LEVEL_WARN, "Disconnect // impossible : >>, Packet Type : REQ_MESSAGE");
				Disconnect(sessionId);
				return;
			}

			// Player �˻�
			playerMapLock.SharedLock();
			Player* p_player = playerMap.find(sessionId)->second;
			playerMapLock.ReleaseSharedLock();

			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE;
			*p_packet << (INT64)accountNo;
			p_packet->PutData((char*)p_player->id, ID_LEN * 2);
			p_packet->PutData((char*)p_player->nickname, NICKNAME_LEN * 2);
			*p_packet << (WORD)msgLen;
			p_packet->PutData((char*)msg, msgLen);
			SendSectorAround(p_player, p_packet);
			PacketBuffer::Free(p_packet);
			break;
		}
		default: {
			LOG("ChatServerMT", LOG_LEVEL_WARN, "INVALID Packet type : %d", type);
			Disconnect(sessionId);
			return;
		}
	}
	InterlockedIncrement((LONG*)&updateCount);
}

void ChatServerMT::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	// 9���� Lock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		sectorLock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x].SharedLock();
	}
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		SendSector(send_packet, p_player->sectorAround.around[i]);
	}
	// 9���� Unlock
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		sectorLock[p_player->sectorAround.around[i].y][p_player->sectorAround.around[i].x].ReleaseSharedLock();
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