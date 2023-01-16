#include "ChattingServer.h"
#include "CommonProtocol.h"
using namespace J_LIB;
using namespace std;

#define MAX_GQCS_DECUEING 100

ChattingServer::ChattingServer() {
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (h_iocp == INVALID_HANDLE_VALUE) CRASH();
	updateThread = thread([this]() {UpdateFunc(); });
	// ��Ʈ��Ʈ ������ ����
}

ChattingServer::~ChattingServer(){
}

bool ChattingServer::OnConnectionRequest(in_addr ip, WORD port) {
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void ChattingServer::OnClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc();
	p_player->Set(session_id);
	player_map.insert({ session_id, p_player });
	return;
}

void ChattingServer::OnClientLeave(SESSION_ID session_id) {
	// player map ���� �˻�
	// player map ���� player ����
	// free (player)
}

void ChattingServer::HeartbeatFunc() {
	for (;;) {
		// Sleep(10��)
		// curtime = timegettime();
		// 
		// Player map ��ȸ
		// 
		// if (curtime - ���� ������ ���) > 40 ) // �α��� ��
		//		Disconnect Player

		// if (curtime - ���� ������ ���) > 1 ) // �� �α��� ��
		//		Disconnect Player
	}
}

void ChattingServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	// JOB.id = id;
	// JOB.packet = packet;
	// packet.refCount++;

	// PQCS(&JOB), ä�� ���� �Ϸ� ť (IOCP) ť��
}

void ChattingServer::UpdateFunc() {
	for (;;) {
		OVERLAPPED_ENTRY dequeueEntry[MAX_GQCS_DECUEING];
		LONG dequeueSize;

		BOOL ret_GQCS = GetQueuedCompletionStatusEx(h_iocp, dequeueEntry, MAX_GQCS_DECUEING, (PULONG)&dequeueSize, INFINITE, FALSE);

		for (int i = 0; i < dequeueSize; i++) {
			// JOB* job = dequeueEntry[i].lpCompletionKey;
			// if (job == nullptr) ���� ����

			// job->id �� player* �˻�
			// job->packet >> packet_type;
			// ProcPacket(player*, packet_type, job->packet);
			// Free( job->packet )
		}
	}
}

bool ChattingServer::ProcPacket(Player* p_player, WORD type, PacketBuffer* cs_contentsPacket) {
	switch (type) {
		case en_PACKET_CS_CHAT_REQ_LOGIN: 
			return ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(p_player, cs_contentsPacket);
		
		// ...

		default:
			break;
	}

	return false;
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(Player* p_player, PacketBuffer* cs_contentsPacket){
	// cs_contentsPacket >> AccountNo, ID, NickName, SessionKey
	// ������ū == true
	//		ID, Nickname Set
	//		�α��� ���� ��Ŷ Send
	return true;
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket){
	// �α��� ���� üũ -> �α��� ���� ���� ���� ��� Disconnect
	// Sector X,Y ��ǥ Ȯ�� (-1 or ���� �ʰ� or Sector ��ǥ ����) Disconnect
	// �÷��̾� Sector �ֽ�ȭ
	// ���� �̵���� ��Ŷ Send
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket){
	// �α��� ���� üũ -> �α��� ���� ���� ���� ��� Disconnect
	// ����ȭ ��Ŷ ���� << �޽���
	// ä�� ������ (�ֺ�����)
	return false;
}

bool ChattingServer::ProcPacket_en_PACKET_CS_CHAT_REQ_HEARTBEATE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket){
	// �α��� ���� üũ -> �α��� ���� ���� ���� ��� Disconnect
	// ������ ��Žð� �ֽ�ȭ
	return false;
}

void ChattingServer::Disconnect_Player(Player* p_player) {

}

void ChattingServer::OnError(int errorcode) {
}
