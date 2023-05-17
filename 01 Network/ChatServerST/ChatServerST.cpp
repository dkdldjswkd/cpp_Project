#include "ChatServerST.h"
#include "CommonProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include "../../00 lib_jy/Profiler.h"
using namespace std;

// 프로파일러 제거
#define PRO_BEGIN(TagName)
#define PRO_END(TagName)
#define PRO_RESET()		
#define PRO_FILEOUT()

#define MAX_MSG 300

ChatServerST::ChatServerST(const char* systemFile, const char* server) : NetServer(systemFile, server) {
	updateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	updateThread = thread([this] { UpdateFunc(); });
#if ON_LOGIN
	authEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	authThread = thread([this] { AuthFunc(); });
	redisClient.connect();
#endif
}

ChatServerST::~ChatServerST() {
}

void ChatServerST::OnServerStop() {
	// 채팅서버 종료
	while (0 < jobQ.GetUseCount()) Sleep(100);
#if ON_LOGIN
	while (0 < tokenQ.GetUseCount()) Sleep(100);
	// 더블체크 (Auth 스레드에서의 Queuing 가능성)
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

void ChatServerST::OnClientJoin(SessionId sessionId) {
	JobQueuing(sessionId, JOB_TYPE_CLIENT_JOIN);
}

void ChatServerST::OnClientLeave(SessionId sessionId) {
	JobQueuing(sessionId, JOB_TYPE_CLIENT_LEAVE);
}

void ChatServerST::OnRecv(SessionId sessionId, PacketBuffer* csContentsPacket) {
	WORD type;
	try {
		*csContentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // impossible : >> type");
		Disconnect(sessionId);
		return;
	}

	// Packet type : 2, 4, 5 (INVALID PACKET TYPE)
	if ((5 < type) || (type == 2) || (type == 4)) {
		LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // OnRecv() : INVALID Packet type (%d)", type);
		Disconnect(sessionId);
		return;
	}

	// Packet type : 1, 3, 5 (VALID PACKET TYPE)
	csContentsPacket->IncrementRefCount();
	JobQueuing(sessionId, type, csContentsPacket);
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
			SessionId sessionId = p_job->sessionId;
			PacketBuffer* csContentsPacket = p_job->p_packet;

			switch (p_job->type) {
				case JOB_TYPE_CLIENT_JOIN: {
					Player* p_player = playerPool.Alloc();
					p_player->Set(sessionId);
					playerMap.insert({ sessionId, p_player });
					break;
				}
				case JOB_TYPE_CLIENT_LEAVE: {
					// Player 검색
					auto iter = playerMap.find(sessionId);
					Player* p_player = iter->second;

					// 컨테이너에서 삭제
					playerMap.erase(iter);

					// Sector 에서 삭제
					if (!p_player->sectorPos.IsInvalid()) {
						sectorSet[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
					}

					// Player 반환
					if (p_player->isLogin) playerCount--;
					p_player->Reset();
					playerPool.Free(p_player);
					break;
				}
				case en_PACKET_CS_CHAT_REQ_LOGIN: {
					auto iter = playerMap.find(sessionId);
					if (iter == playerMap.end()) {
						LOG("ChatServerST", LOG_LEVEL_FATAL, "REQ_LOGIN() : player_map.find(sessionId) == player_map.end()");
						PacketBuffer::Free(csContentsPacket);
						break;
					}
					Player* p_player = iter->second;

					// 이미 로그인 된 플레이어
					if (p_player->isLogin) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // Already login!!");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						break;
					}

					#if ON_LOGIN
					AccountToken* p_at = tokenPool.Alloc();
					p_at->sessionId = sessionId;
					try {
						csContentsPacket->GetData((char*)&p_at->accountNo, sizeof(AccountNo));
						csContentsPacket->GetData((char*)p_player->id, ID_LEN * 2);
						csContentsPacket->GetData((char*)p_player->nickname, NICKNAME_LEN * 2);
						csContentsPacket->GetData((char*)&p_at->token, 64);
					}
					catch (const PacketException& e) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // impossible : >> Login packet");
						tokenPool.Free(p_at);
						Disconnect(sessionId);
						break;
					}
					p_player->accountNo = p_at->accountNo;
					tokenQ.Enqueue(p_at);
					SetEvent(authEvent);
					#else ON_LOGIN
					try {
						*csContentsPacket >> p_player->accountNo;
						csContentsPacket->GetData((char*)p_player->id, ID_LEN * 2);
						csContentsPacket->GetData((char*)p_player->nickname, NICKNAME_LEN * 2);
						Token token; // 사용하지 않음
						csContentsPacket->GetData((char*)&token, 64);
					}
					catch (const PacketException& e) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // impossible : >> Login packet");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						break;
					}

					// 로그인 성공
					p_player->isLogin = true;
					playerCount++;

					// 로그인 응답 패킷 회신
					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
					*p_packet << (BYTE)p_player->isLogin;
					*p_packet << (INT64)p_player->accountNo;
					SendPacket(sessionId, p_packet);
					PacketBuffer::Free(p_packet);
					#endif
					PacketBuffer::Free(csContentsPacket);
					break;
				}
				case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE: {
					auto iter = playerMap.find(sessionId);
					if (iter == playerMap.end()) {
						LOG("ChatServerST", LOG_LEVEL_FATAL, "REQ_SECTOR_MOVE() : player_map.find(sessionId) == player_map.end()");
						PacketBuffer::Free(csContentsPacket);
						break;
					}
					Player* p_player = iter->second;

					// 로그인 상태가 아닌 플레이어
					if (!p_player->isLogin) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // is not login!!");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						break;
					}

					INT64 accountNo;
					Sector curSector;
					try {
						*csContentsPacket >> accountNo;
						*csContentsPacket >> curSector.x;
						*csContentsPacket >> curSector.y;
					}
					catch (const PacketException& e) {
						//LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // impossible : >> sector move packet");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						break;
					}

					// 유효하지 않은 섹터로 이동 시도
					if (curSector.IsInvalid()) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // sector move Packet : Invalid Sector!!");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						break;
					}

					// 이 전에 위치하던 Sector erase
					if (!p_player->sectorPos.IsInvalid()) {
						sectorSet[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
					}
					p_player->SetSector(curSector);
					sectorSet[curSector.y][curSector.x].insert(p_player);

					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
					*p_packet << (INT64)accountNo;
					*p_packet << (WORD)curSector.x;
					*p_packet << (WORD)curSector.y;
					SendPacket(sessionId, p_packet);
					PacketBuffer::Free(p_packet);
					PacketBuffer::Free(csContentsPacket);
					break;
				}
				case en_PACKET_CS_CHAT_REQ_MESSAGE: {
					PRO_BEGIN("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
					Player* p_player = playerMap.find(sessionId)->second;
					if (nullptr == p_player) {
						PacketBuffer::Free(csContentsPacket);
						PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
						break;
					}

					// 로그인 상태가 아닌 플레이어
					if (!p_player->isLogin) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // is not login!!");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
						break;
					}

					INT64	accountNo;
					WORD	msgLen;
					WCHAR	msg[MAX_MSG]; // null 미포함
					try {
						*csContentsPacket >> accountNo;
						*csContentsPacket >> msgLen;
						csContentsPacket->GetData((char*)msg, msgLen);
					}
					catch (const PacketException& e) {
						LOG("ChatServerST", LOG_LEVEL_WARN, "Disconnect // impossible : >> chatting packet");
						Disconnect(sessionId);
						PacketBuffer::Free(csContentsPacket);
						PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
						break;
					}
					msg[msgLen / 2] = 0;

					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE;
					*p_packet << (INT64)accountNo;
					p_packet->PutData((char*)p_player->id, ID_LEN * 2);
					p_packet->PutData((char*)p_player->nickname, NICKNAME_LEN * 2);
					*p_packet << (WORD)msgLen;
					p_packet->PutData((char*)msg, msgLen);

					PRO_BEGIN("SendSectorAround");
					SendSectorAround(p_player, p_packet);
					PRO_END("SendSectorAround");
					PacketBuffer::Free(p_packet);
					PacketBuffer::Free(csContentsPacket);
					PRO_END("UpdateFunc::en_PACKET_CS_CHAT_REQ_MESSAGE");
					break;
				}
				case JOB_TYPE_CLIENT_LOGIN_SUCCESS: {
					auto iter = playerMap.find(sessionId);
					if (iter == playerMap.end()) { // 로그인 결과 패킷 회신 전 연결끊킴
						break;
					}
					Player* p_player = iter->second;

					// 로그인 성공
					p_player->isLogin = true;
					playerCount++;

					// 로그인 응답 패킷(성공) 회신
					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
					*p_packet << (BYTE)p_player->isLogin;
					*p_packet << (INT64)p_player->accountNo;
					SendPacket(sessionId, p_packet);
					PacketBuffer::Free(p_packet);
					break;
				}
				case JOB_TYPE_CLIENT_LOGIN_FAIL: {
					auto iter = playerMap.find(sessionId);
					if (iter == playerMap.end()) {  // 로그인 결과 패킷 회신 전 연결끊킴
						break;
					}
					Player* p_player = iter->second;

					// 로그인 응답 패킷(실패) 회신
					PacketBuffer* p_packet = PacketBuffer::Alloc();
					*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
					*p_packet << (BYTE)false;
					*p_packet << (INT64)p_player->accountNo;
					SendPacket(sessionId, p_packet);
					PacketBuffer::Free(p_packet);
					break;
				}
				default: {
					LOG("ChatServerST", LOG_LEVEL_FATAL, "Disconnect // ProcJob INVALID Packet type : %d", p_job->type);
					Disconnect(sessionId);
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
			snprintf(chattingKey, 100, "%lld.chatting", p_at->accountNo);

			std::future<cpp_redis::reply> futureReply = redisClient.get(chattingKey);
			redisClient.sync_commit();
			cpp_redis::reply reply = futureReply.get();
			if (reply.is_string()) {
				memcpy((char*)&redisToken, reply.as_string().c_str(), sizeof(Token));
			}

			// 유효 세션
			if (0 == strncmp(redisToken.buf, p_at->token.buf, 64)) {
				redisClient.del({ chattingKey });
				redisClient.sync_commit();
				JobQueuing(p_at->sessionId, JOB_TYPE_CLIENT_LOGIN_SUCCESS);
			}
			// 유효하지 않은 세션
			else {
				JobQueuing(p_at->sessionId, JOB_TYPE_CLIENT_LOGIN_FAIL);
				LOG("ChatServerST", LOG_LEVEL_WARN, "INVALID TOKEN AccountNo(%d) Redis(%.*s), User(%.*s)", p_at->accountNo, 64, redisToken.buf, 64, p_at->token.buf);
			}
			tokenPool.Free(p_at);
		}
	}
}

//////////////////////////////
// OTHER
//////////////////////////////

void ChatServerST::JobQueuing(SessionId sessionId, WORD type, PacketBuffer* p_packet) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(sessionId, type, p_packet);
	jobQ.Enqueue(p_job);
	SetEvent(updateEvent);
}

void ChatServerST::JobQueuing(SessionId sessionId, WORD type) {
	Job* p_job = jobPool.Alloc();
	p_job->Set(sessionId, type);
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
		for (auto iter = sectorSet[sector.y][sector.x].begin(); iter != sectorSet[sector.y][sector.x].end(); ++iter) {
			auto p_player = *iter;
			SendPacket(p_player->sessionId, send_packet);
		}
	}
}