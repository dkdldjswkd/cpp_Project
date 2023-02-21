#include "EchoServer.h"

// 오후 5:29 2023-02-21

EchoGameServer::EchoGameServer(const char* systemFile, const char* server) : NetworkLib(systemFile, server){
}

EchoGameServer::~EchoGameServer() {
}

void EchoGameServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	//------------------------------
	// var set
	//------------------------------
	PacketBuffer* sc_contentsPacket = PacketBuffer::Alloc();

	//------------------------------
	// SC Contents Packet 생성
	//------------------------------
	auto cs_contentsPacket_len = cs_contentsPacket->Get_PayloadSize();
	sc_contentsPacket->Put_Data(cs_contentsPacket->Get_payloadPos(), cs_contentsPacket_len);
	cs_contentsPacket->Move_Rp(cs_contentsPacket_len);

	//------------------------------
	// Send Packet
	//------------------------------
	NetworkLib::SendPacket(session_id, sc_contentsPacket);

	//------------------------------
	// Release Func
	//------------------------------
	PacketBuffer::Free(sc_contentsPacket);
	return;
}

bool EchoGameServer::OnConnectionRequest(in_addr ip, WORD port){
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void EchoGameServer::OnClientJoin(SESSION_ID session_id){
	//------------------------------
	// var set
	//------------------------------
	PacketBuffer* sc_packet = PacketBuffer::Alloc();

	//------------------------------
	// sc packet 조립
	//------------------------------
	*sc_packet << 0x7fffffffffffffff;

	//------------------------------
	// Send Packet
	//------------------------------
	NetworkLib::SendPacket(session_id, sc_packet);

	//------------------------------
	// Release Func
	//------------------------------
	PacketBuffer::Free(sc_packet);
	return;
}

void EchoGameServer::OnClientLeave(SESSION_ID session_id){
}

void EchoGameServer::OnError(int errorcode){
}