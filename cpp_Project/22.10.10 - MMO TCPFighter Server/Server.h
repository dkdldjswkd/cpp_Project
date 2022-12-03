#pragma once
#include <WinSock2.h>
#include <unordered_map>
#include "Session.h"
#include "../../J_LIB/ObjectPool/ObjectPool.h"
#include "../../J_LIB/ProtocolBuffer/ProtocolBuffer.h"

struct Character;
struct SectorAround;

extern SOCKET listen_sock;
extern std::unordered_map<SOCKET, Session*>			session_map;
extern std::unordered_map<SESSION_ID, Character*>	character_map;

// Network
struct Network{
	static bool StartUp();
	static bool CleanUp();
	static void Process_NetIO();
	static void Select_Socket(SOCKET* p_SocketTable, FD_SET* p_ReadSet, FD_SET* p_WriteSet);
	static void Proc_Recv(SOCKET sock);
	static bool Proc_Send(SOCKET sock);
	static bool Proc_Accept();
	static void Disconnect_Session();
};

// Send
void SendPacket_Unicast(Session* p_session, ProtocolBuffer* p_packet);
void SendPacket_SectorOne(int sector_x, int sector_y, ProtocolBuffer* p_packet, Session* p_except_session);
void SendPacket_Around(SectorAround* p_sector_around, ProtocolBuffer* p_packet, Session* p_except_session = nullptr);
//void SendPacket_Around(Session* p_session, ProtocolBuffer* p_packet, Session* p_except_session = nullptr);

// CS 패킷 처리 (수동적 패킷 Send)
int	 Complete_RecvPacket(Session* p_session);
bool Proc_Packet(Session* p_session, unsigned char packet_type, ProtocolBuffer* p_packet);
bool Proc_Packet_MOVE_START	(Session* p_session, ProtocolBuffer* cs_payload);
bool Proc_Packet_MOVE_STOP	(Session* p_session, ProtocolBuffer* cs_payload);
bool Proc_Packet_ATTACK1(Session* p_session, ProtocolBuffer* cs_payload);
bool Proc_Packet_ATTACK2(Session* p_session, ProtocolBuffer* cs_payload);
bool Proc_Packet_ATTACK3(Session* p_session, ProtocolBuffer* cs_payload);
bool Proc_Packet_Echo		(Session* p_session, ProtocolBuffer* cs_payload);

// SC 패킷 (능동적 패킷 Send)
struct SCPacket {
	static void CREATE_MY_CHARACTER(Character* p_character);
	static void CharacterSectorUpdate(Character* p_character);
	static void Disconnect_Character(Character* p_character);
};

// 컨텐츠
void Update(DWORD cur_time);