#include "EchoServer.h"
using namespace J_LIB;

#define ECHO_ON
#ifdef ECHO_ON

void EchoServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket) {
	//------------------------------
	// var set
	//------------------------------
	PacketBuffer* sc_contentsPacket = PacketBuffer::Alloc_LanPacket();

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

bool EchoServer::OnConnectionRequest(in_addr ip, WORD port){
	//printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void EchoServer::OnClientJoin(SESSION_ID session_id){
	//------------------------------
	// var set
	//------------------------------
	PacketBuffer* sc_packet = PacketBuffer::Alloc_LanPacket();

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

void EchoServer::OnClientLeave(SESSION_ID session_id){
}

void EchoServer::OnError(int errorcode){
}

#endif