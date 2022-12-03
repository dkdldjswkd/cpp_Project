#include "stdafx.h"
#include "MakePacket.h"
#include "Protocol.h"
#include "Define.h"
#include "Log.h"

#include "../../J_LIB/ProtocolBuffer/ProtocolBuffer.h"
#pragma comment(lib, "../../J_LIB/ProtocolBuffer/ProtocolBuffer.lib")

void MakePacket_CREATE_MY_CHARACTER(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y, char hp){
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 10;
	packet_header.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)dir;
	(*p_packet) << x;
	(*p_packet) << y;
	(*p_packet) << hp;
}

void MakePacket_CREATE_OTHER_CHARACTER(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y, char hp){
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 10;
	packet_header.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)dir;
	(*p_packet) << x;
	(*p_packet) << y;
	(*p_packet) << hp;
}

void MakePacket_DELETE_CHARACTER(ProtocolBuffer* p_packet, SESSION_ID session_id) {
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 4;
	packet_header.byType = dfPACKET_SC_DELETE_CHARACTER;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
}

void MakePacket_MOVE_START(ProtocolBuffer* p_packet, SESSION_ID session_id, ACTION action, short x, short y) {
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 9;
	packet_header.byType = dfPACKET_SC_MOVE_START;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)action;
	(*p_packet) << x;
	(*p_packet) << y;
}

void MakePacket_MOVE_STOP(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y){
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 9;
	packet_header.byType = dfPACKET_SC_MOVE_STOP;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)dir;
	(*p_packet) << x;
	(*p_packet) << y;
}

void MakePacket_ATTACK1(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y){
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 9;
	packet_header.byType = dfPACKET_SC_ATTACK1;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)dir;
	(*p_packet) << x;
	(*p_packet) << y;
}

void MakePacket_ATTACK2(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y) {
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 9;
	packet_header.byType = dfPACKET_SC_ATTACK2;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)dir;
	(*p_packet) << x;
	(*p_packet) << y;
}

void MakePacket_ATTACK3(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y) {
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 9;
	packet_header.byType = dfPACKET_SC_ATTACK3;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << (BYTE)dir;
	(*p_packet) << x;
	(*p_packet) << y;
}

void MakePacket_DAMAGE(ProtocolBuffer* p_packet, SESSION_ID attack_id, SESSION_ID damage_id, char damage_hp){
	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 9;
	packet_header.byType = dfPACKET_SC_DAMAGE;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << attack_id;
	(*p_packet) << damage_id;
	(*p_packet) << damage_hp;
}

void MakePacket_SYNC(ProtocolBuffer* p_packet, SESSION_ID session_id, short x, short y){
	// 디버깅
	syncMsg_count++;
	//Log::Warn("SYNC");

	//--------------------------------
	// 헤더 쌓기
	//--------------------------------

	PACKET_HEADER packet_header;
	packet_header.byCode = PACKET_CODE;
	packet_header.bySize = 8;
	packet_header.byType = dfPACKET_SC_SYNC;
	p_packet->Put_Data((char*)&packet_header, sizeof(packet_header));

	//--------------------------------
	// 페이로드 쌓기
	//--------------------------------

	(*p_packet) << session_id;
	(*p_packet) << x;
	(*p_packet) << y;
}