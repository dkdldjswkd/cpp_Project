//#include "ChattingServer.h"
//#include "CommonProtocol.h"
//using namespace J_LIB;
//using namespace std;
//
//#define MAX_MSG 300
//
//ChattingServer::ChattingServer() {
//	updateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	updateThread = thread([this]{UpdateFunc(); });
//}
//
//ChattingServer::~ChattingServer(){
//}
//
//// Session Connect 시
//bool ChattingServer::OnConnectionRequest(in_addr ip, WORD port) {
//	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
//	return true;
//}
// 
//// Session 할당 후
//void ChattingServer::OnClientJoin(SESSION_ID session_id) {
//	JobQueuing(session_id, JOB_TYPE_CLIENT_JOIN);
//}
//
//void ChattingServer::OnClientLeave(SESSION_ID session_id) {
//	JobQueuing(session_id, JOB_TYPE_CLIENT_LEAVE);
//}
//
//void ChattingServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
//	WORD type;
//	*cs_contentsPacket >> type;
//	cs_contentsPacket->Increment_refCount();
//	JobQueuing(session_id, type, cs_contentsPacket);
//}
//
//void ChattingServer::UpdateFunc() {
//	Job* p_job;
//	for (;;) {
//		WaitForSingleObject(updateEvent, INFINITE);
//		while (jobQ.Dequeue(&p_job)) {
//			ProcJob(p_job->session_id, p_job->type, p_job->p_packet);
//			if (nullptr != p_job->p_packet) { PacketBuffer::Free(p_job->p_packet); }
//			jobPool.Free(p_job);
//		}
//	}
//}
//
//bool ChattingServer::ProcJob(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket) {
//	switch (type) {
//		// 패킷처리
//		case en_PACKET_CS_CHAT_REQ_LOGIN:
//			return ProcJob_en_PACKET_CS_CHAT_REQ_LOGIN(session_id, cs_contentsPacket);
//
//		case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
//			return ProcJob_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(session_id, cs_contentsPacket);
//
//		case en_PACKET_CS_CHAT_REQ_MESSAGE:
//			return ProcJob_en_PACKET_CS_CHAT_REQ_MESSAGE(session_id, cs_contentsPacket);
//
//		// OnFunc 처리
//		case JOB_TYPE_CLIENT_JOIN:
//			return ProcJob_ClientJoin(session_id);
//
//		case JOB_TYPE_CLIENT_LEAVE:
//			return ProcJob_ClientLeave(session_id);
//
//		default: {
//			CRASH();
//		}
//	}
//}
//
//bool ChattingServer::ProcJob_en_PACKET_CS_CHAT_REQ_LOGIN(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
//	INT64 accountNo;
//
//	auto iter = player_map.find(session_id);
//	if (player_map.end() == iter) {
//		CRASH(); // end 일 수 없음
//	}
//	Player* p_player = iter->second;
//
//	*cs_contentsPacket >> accountNo;
//	if (false == p_player->is_login) {
//		// 로그인 성공
//		p_player->Set_Login();
//
//		cs_contentsPacket->Get_Data((char*)p_player->id, ID_LEN * 2);
//		cs_contentsPacket->Get_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
//		cs_contentsPacket->Get_Data((char*)p_player->sessionKey, 64); // 인증키 확인해야함
//	}
//
//	// 로그인 응답 패킷 회신
//	PacketBuffer* p_packet = PacketBuffer::Alloc_NetPacket();
//	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN;
//	*p_packet << (BYTE)p_player->is_login;
//	*p_packet << (INT64)accountNo;
//	SendPacket(session_id, p_packet);
//	PacketBuffer::Free(p_packet);
//
//	return p_player->is_login;
//}
//
//bool ChattingServer::ProcJob_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket) {
//	auto iter = player_map.find(session_id);
//	if (player_map.end() == iter) { 
//		CRASH(); // end 일 수 없음
//	}
//	Player* p_player = iter->second;
//	if (false == p_player->is_login) {
//		CRASH(); // Disconnect 해줘야함. 눈으로 먼저 보자 
//	}
//
//	INT64 accountNo;
//	WORD sectorX;
//	WORD sectorY;
//	*cs_contentsPacket >> accountNo;
//	*cs_contentsPacket >> sectorX;
//	*cs_contentsPacket >> sectorY;
//
//	// 섹터 좌표 조건 체크 안하고있음
//	if (p_player->sectorPos.CheckInvalid() == false) {
//		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
//	}
//	p_player->Set_Sector({ sectorX , sectorY }); 
//	sectors_set[sectorY][sectorX].insert(p_player);
//
//	PacketBuffer* p_packet = PacketBuffer::Alloc_NetPacket();
//	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
//	*p_packet << (INT64)accountNo;
//	*p_packet << (WORD)sectorX;
//	*p_packet << (WORD)sectorY;
//	SendPacket(session_id, p_packet);
//	PacketBuffer::Free(p_packet);
//
//	return true;
//}
//
//bool ChattingServer::ProcJob_en_PACKET_CS_CHAT_REQ_MESSAGE(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket){
//	auto iter = player_map.find(session_id);
//	if (player_map.end() == iter) {
//		CRASH(); // end 일 수 없음
//	}
//	Player* p_player = iter->second;
//	if (false == p_player->is_login) {
//		CRASH(); // Disconnect 해줘야함. 눈으로 먼저 보자 
//	}
//
//	// >>
//	INT64	accountNo;
//	WORD	msgLen;
//	WCHAR	msg[MAX_MSG]; // null 미포함
//	*cs_contentsPacket >> accountNo;
//	*cs_contentsPacket >> msgLen; 
//	cs_contentsPacket->Get_Data((char*)msg, msgLen);
//	msg[msgLen / 2] = 0;
//
//	// <<
//	PacketBuffer* p_packet = PacketBuffer::Alloc_NetPacket();
//	*p_packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE;
//	*p_packet << (INT64)accountNo;
//	p_packet->Put_Data((char*)p_player->id, ID_LEN * 2);
//	p_packet->Put_Data((char*)p_player->nickname, NICKNAME_LEN * 2);
//	*p_packet << (WORD)msgLen;
//	p_packet->Put_Data((char*)msg, msgLen);
//
//	SendSectorAround(p_player, p_packet);
//	PacketBuffer::Free(p_packet);
//	return true;
//}
//
//// Player 객체 생성 (로그인 상태 X)
//bool ChattingServer::ProcJob_ClientJoin(SESSION_ID session_id) {
//	Player* p_player = playerPool.Alloc();
//	p_player->Set_Connect(session_id);
//	player_map.insert({ session_id, p_player });
//	player_count++;
//	return true;
//}
//
//bool ChattingServer::ProcJob_ClientLeave(SESSION_ID session_id) {
//	// Player 검색
//	auto iter = player_map.find(session_id);
//	IF_CRASH(iter == player_map.end());
//	Player* p_player = iter->second;
//
//	// 컨테이너에서 삭제
//	player_map.erase(iter);
//	if (p_player->sectorPos.CheckInvalid() == false) {
//		sectors_set[p_player->sectorPos.y][p_player->sectorPos.x].erase(p_player);
//	}
//
//	// Player 반환
//	p_player->Reset();
//	playerPool.Free(p_player);
//	return false;
//}
//
//void ChattingServer::Disconnect_Player(Player* p_player) {
//}
//
//void ChattingServer::OnError(int errorcode) {
//}
//
//void ChattingServer::JobQueuing(SESSION_ID session_id, WORD type, J_LIB::PacketBuffer* p_packet) {
//	Job* p_job = jobPool.Alloc();
//	p_job->Set(session_id, type, p_packet);
//	jobQ.Enqueue(p_job);
//	SetEvent(updateEvent);
//}
//
//void ChattingServer::SendSectorAround(Player* p_player, PacketBuffer* send_packet) {
//	for (int i = 0; i < p_player->sectorAround.count; i++) {
//		SendSector(send_packet, p_player->sectorAround.around[i]);
//	}
//}
//
//void ChattingServer::SendSector(PacketBuffer* send_packet, Sector sector) {
//	auto iter = sectors_set[sector.y][sector.x].begin();
//	for (; iter != sectors_set[sector.y][sector.x].end(); iter++) {
//		SendPacket((*iter)->session_id, send_packet);
//	}
//}