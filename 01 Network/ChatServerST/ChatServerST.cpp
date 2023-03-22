#include "ChatServerST.h"
#include "CommonProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include "../../00 lib_jy/Profiler.h"
#include "../../00 lib_jy/ThreadCpuMonitor.h"
using namespace std;

// �������Ϸ� ����
#define PRO_BEGIN(TagName)
#define PRO_END(TagName)
#define PRO_RESET()		
#define PRO_FILEOUT()

#define MAX_MSG 300

ChatServerST::ChatServerST(const char* systemFile, const char* server) : NetServer(systemFile, server) {
	updateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	updateThread = thread([this] {UpdateFunc(); });
#if ON_LOGIN
	authEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	authThread = thread([this] {AuthFunc(); });
	connectorRedis.connect();
#endif
}

ChatServerST::~ChatServerST() {
}

void ChatServerST::OnServerStop() {
	// ä�ü��� ����
	while (0 < jobQ.GetUseCount()) Sleep(100);
#if ON_LOGIN
	while (0 < tokenQ.GetUseCount()) Sleep(100);
	// ����üũ (Auth �����忡���� Queuing ���ɼ�)
	while (0 < jobQ.GetUseCount()) Sleep(100);
#endif

	UpdateStop();
	if (updateThread.joinable())
		updateThread.join();
#if ON_LOGIN
	AuthStop();
	if(authThread.joinable())
		authThread.join();
#endif
}

void ChatServerST::OnClientJoin(SESSION_ID session_id) {
	JobQueuing(session_id, JOB_TYPE_CLIENT_JOIN);
}

void ChatServerST::OnClientLeave(SESSION_ID session_id) {
	JobQueuing(session_id, JOB_TYPE_CLIENT_LEAVE);
}

void ChatServerST::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	WORD type;
	try {
		*cs_contentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> type");
		Disconnect(session_id);
		return;
	}

	// Packet type : 2, 4, 5 (INVALID PACKET TYPE)
	if ((5 < type) || (type == 2) || (type == 4)) {
		LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // OnRecv() : INVALID Packet type (%d)", type);
		Disconnect(session_id);
		return;
	}

	// Packet type : 1, 3, 5 (VALID PACKET TYPE)
	cs_contentsPacket->IncrementRefCount();
	JobQueuing(session_id, type, cs_contentsPacket);
}

void ChatServerST::UpdateFunc() {
	Job* p_job;
	for (;;) {
		WaitForSingleObject(updateEvent, INFINITE);
		if (updateStop) break;
		for (;;) {
			if (jobQ.GetUseCount() < 1) break;
			jobQ.Dequeue(&p_job);

			PRO_BEGIN("UpdateFunc");
			SESSION_ID session_id = p_job->session_id;
			PacketBuffer* cs_contentsPacket = p_job->p_packet;

			switch (p_job->type) {
				case JOB_TYPE_CLIENT_JOIN: {
					Player* p_player = playerPool.Alloc();
					p_player->Set(session_id);
					player_map.insert({ session_id, p_player });
					break;
				}
				case JOB_TYPE_CLIENT_LEAVE: {
					// Player �˻�
					auto iter = player_map.find(session_id);
					Player* p_player = iter->second;

					// �����̳ʿ��� ����
					player_map.erase(iter);

					// Sector ���� ����
					if (!p_player->sectorPos.Is_Invalid()) {
						sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
					}

					// Player ��ȯ
					if (p_player->isLogin) {
						p_player->Reset();
						playerCount--;
					}
					playerPool.Free(p_player);
					break;
				}
				case en_PACKET_CS_CHAT_REQ_LOGIN: {
					auto iter = player_map.find(session_id);
					if (iter == player_map.end()) {
						LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "REQ_LOGIN() : player_map.find(session_id) == player_map.end()");
						PacketBuffer::Free(cs_contentsPacket);
						break;
					}
					Player* p_player = iter->second;

					// �̹� �α��� �� �÷��̾�
					if (p_player->isLogin) {
						LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // Already login!!");
						Disconnect(session_id);
						PacketBuffer::Free(cs_contentsPacket);
						break;
					}

	#if ON_LOGIN
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
						break;
					}
					p_player->accountNo = p_at->accountNo;
					tokenQ.Enqueue(p_at);
					SetEvent(authEvent);
	#else ON_LOGIN
					try {
						*cs_contentsPacket >> p_player->accountNo;
						cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
						cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
						Token token; // ������� ����
						cs_contentsPacket->Get_Data((char*)&token, 64);
					}
					catch (const PacketException& e) {
						LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> Login packet");
						Disconnect(session_id);
						PacketBuffer::Free(cs_contentsPacket);
						break;
					}

					// �α��� ����
					p_player->isLogin = true;
					playerCount++;

					// �α��� ���� ��Ŷ ȸ��
					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
					*p_packet << (BYTE)p_player->isLogin;
					*p_packet << (INT64)p_player->accountNo;
					SendPacket(session_id, p_packet);
					PacketBuffer::Free(p_packet);
	#endif
					PacketBuffer::Free(cs_contentsPacket);
					break;
				}
				case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE: {
					auto iter = player_map.find(session_id);
					if (iter == player_map.end()) {
						LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "REQ_SECTOR_MOVE() : player_map.find(session_id) == player_map.end()");
						PacketBuffer::Free(cs_contentsPacket);
						break;
					}
					Player* p_player = iter->second;

					// �α��� ���°� �ƴ� �÷��̾�
					if (!p_player->isLogin) {
						LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // is not login!!");
						Disconnect(session_id);
						PacketBuffer::Free(cs_contentsPacket);
						break;
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
						PacketBuffer::Free(cs_contentsPacket);
						break;
					}

					// ��ȿ���� ���� ���ͷ� �̵� �õ�
					if (cur_sector.Is_Invalid()) {
						LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // sector move Packet : Invalid Sector!!");
						Disconnect(session_id);
						PacketBuffer::Free(cs_contentsPacket);
						break;
					}

					// �� ���� ��ġ�ϴ� Sector erase
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
					PacketBuffer::Free(cs_contentsPacket);
					break;
				}
				case en_PACKET_CS_CHAT_REQ_MESSAGE: {
					PRO_BEGIN("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
					Player* p_player = player_map.find(session_id)->second;
					if (nullptr == p_player) {
						PacketBuffer::Free(cs_contentsPacket);
						PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
						break;
					}

					// �α��� ���°� �ƴ� �÷��̾�
					if (!p_player->isLogin) {
						LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // is not login!!");
						Disconnect(session_id);
						PacketBuffer::Free(cs_contentsPacket);
						PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
						break;
					}

					// >>
					INT64	accountNo;
					WORD	msgLen;
					WCHAR	msg[MAX_MSG]; // null ������
					try {
						*cs_contentsPacket >> accountNo;
						*cs_contentsPacket >> msgLen;
						cs_contentsPacket->Get_Data((char*)msg, msgLen);
					}
					catch (const PacketException& e) {
						//LOG("ChattingServer-Single", LOG_LEVEL_WARN, "Disconnect // impossible : >> chatting packet");
						Disconnect(session_id);
						PacketBuffer::Free(cs_contentsPacket);
						PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
						break;
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

					PRO_BEGIN("SendSectorAround");
					SendSectorAround(p_player, p_packet);
					PRO_END("SendSectorAround");
					PacketBuffer::Free(p_packet);
					PacketBuffer::Free(cs_contentsPacket);
					PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
					break;
				}
				case JOB_TYPE_CLIENT_LOGIN_SUCCESS: {
					auto iter = player_map.find(session_id);
					if (iter == player_map.end()) { // �α��� ��� ��Ŷ ȸ�� �� �����Ŵ
						break;
					}
					Player* p_player = iter->second;

					// �α��� ����
					p_player->isLogin = true;
					playerCount++;

					// �α��� ���� ��Ŷ(����) ȸ��
					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
					*p_packet << (BYTE)p_player->isLogin;
					*p_packet << (INT64)p_player->accountNo;
					SendPacket(session_id, p_packet);
					PacketBuffer::Free(p_packet);
					break;
				}
				case JOB_TYPE_CLIENT_LOGIN_FAIL: {
					auto iter = player_map.find(session_id);
					if (iter == player_map.end()) {  // �α��� ��� ��Ŷ ȸ�� �� �����Ŵ
						break;
					}
					Player* p_player = iter->second;

					// �α��� ���� ��Ŷ(����) ȸ��
					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
					*p_packet << (BYTE)false;
					*p_packet << (INT64)p_player->accountNo;
					SendPacket(session_id, p_packet);
					PacketBuffer::Free(p_packet);
					break;
				}
				default: {
					LOG("ChattingServer-Single", LOG_LEVEL_FATAL, "Disconnect // ProcJob INVALID Packet type : %d", p_job->type);
					Disconnect(session_id);
					break;
				}
			}
			updateCount++;
			jobPool.Free(p_job);
			PRO_END("UpdateFunc");
		}
	}
}

void ChatServerST::AuthFunc() {
	Token redisToken;
	AccountToken* p_at;

	for (;;) {
		WaitForSingleObject(authEvent, INFINITE);
		if (authStop) break;

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

			// ��ȿ ����
			if (0 == strncmp(redisToken.buf, p_at->token.buf, 64)) {
				connectorRedis.del({ chattingKey });
				connectorRedis.sync_commit();
				JobQueuing(p_at->sessionID, JOB_TYPE_CLIENT_LOGIN_SUCCESS);
			}
			// ��ȿ���� ���� ����
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

void ChatServerST::JobQueuing(SESSION_ID session_id, WORD type, PacketBuffer* p_packet) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(session_id, type, p_packet);
	jobQ.Enqueue(p_job);
	SetEvent(updateEvent);
}

void ChatServerST::JobQueuing(SESSION_ID session_id, WORD type) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(session_id, type);
	jobQ.Enqueue(p_job);
	SetEvent(updateEvent);
}

void ChatServerST::UpdateStop() {
	updateStop = true;
	SetEvent(updateEvent);
}

void ChatServerST::AuthStop() {
	authStop = true;
	SetEvent(authEvent);
}

void ChatServerST::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
	for (int i = 0; i < p_player->sectorAround.count; i++) {
		auto sector = p_player->sectorAround.around[i];
		//SendSector
		for (auto iter = sectors_set[sector.y][sector.x].begin(); iter != sectors_set[sector.y][sector.x].end(); ++iter) {
			auto p_player = *iter;
			SendPacket(p_player->sessionID, send_packet);
		}
	}
}