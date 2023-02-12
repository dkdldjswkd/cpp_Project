#pragma once
#include "Session.h"

struct ProtocolBuffer;
enum class ACTION : BYTE;
enum class DIR : BYTE;

void MakePacket_CREATE_MY_CHARACTER(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y, char hp);
void MakePacket_CREATE_OTHER_CHARACTER(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y, char hp);
void MakePacket_DELETE_CHARACTER(ProtocolBuffer* p_packet, SESSION_ID session_id);
void MakePacket_MOVE_START(ProtocolBuffer* p_packet, SESSION_ID session_id, ACTION action, short x, short y);
void MakePacket_MOVE_STOP(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y);
void MakePacket_ATTACK1(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y);
void MakePacket_ATTACK2(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y);
void MakePacket_ATTACK3(ProtocolBuffer* p_packet, SESSION_ID session_id, DIR dir, short x, short y);
void MakePacket_DAMAGE(ProtocolBuffer* p_packet, SESSION_ID attack_id, SESSION_ID damage_id, char damage_hp);
void MakePacket_SYNC(ProtocolBuffer* p_packet, SESSION_ID session_id, short x, short y);