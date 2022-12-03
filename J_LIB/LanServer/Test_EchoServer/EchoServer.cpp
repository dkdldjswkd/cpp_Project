#include "EchoServer.h"
#include "ProtocolBuffer.h"
#pragma comment(lib, "../LanServer.lib")
#pragma comment(lib, "RingBuffer.lib")
#pragma comment(lib, "ws2_32.lib")

void EchoServer::OnRecv(SESSION_ID session_id, ProtocolBuffer* cs_contentsPacket) {
	printf("asdf \n");

	////------------------------------
	//// var set
	////------------------------------
	//ProtocolBuffer sc_contentsPacket;

	////------------------------------
	//// SC Contents Packet »ý¼º
	////------------------------------
	//auto cs_contentsPacket_len = cs_contentsPacket->Get_UseSize();
	//sc_contentsPacket.Put_Data(cs_contentsPacket->Get_readPos(), cs_contentsPacket_len);
	//cs_contentsPacket->Move_Rp(cs_contentsPacket_len);

	////------------------------------
	//// Send Packet
	////------------------------------
	//LanServer::Send_Packet(session_id, &sc_contentsPacket);

	////------------------------------
	//// Release Func
	////------------------------------
	//return;
}