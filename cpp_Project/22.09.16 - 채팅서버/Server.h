#pragma once
#include "../../J_LIB/RingBuffer/RingBuffer.h"
#pragma comment(lib, "../../J_LIB/RingBuffer/RingBuffer.lib")
#include <queue>
#include <map>
#include <vector>
#include <list>
#include "Packet.h"
#include "NetworkDefine.h"
#include "Session.h"

typedef int session_id;
typedef int room_no;

// Session
extern std::map<int, Session*> map_session_infos;

// ·¦ÇÎ
bool Server_Init();
bool Server_Run();
bool Server_Close();

// NETWORK
int Unicast(session_id id, PACKET_HEADER* header, const char* packet);
void Broadcast(PACKET_HEADER* header, const char* packet, std::list<session_id>& excludes);
void Broadcast(PACKET_HEADER* header, const char* packet);

void Accept_Proc();
void Recv_Proc(session_id id);
void Send_Proc(session_id id);
void Close_Proc(session_id id);
void NetWork_Process();
void Select_Proc(int* p_table_id, SOCKET* p_table_socket, FD_SET* p_readSet, FD_SET* p_writeSet);

// PAKCET
void PacketProc(session_id id, short msg_type, ProtocolBuffer& cs_payload_buf);
void MakeCheckSum(PACKET_HEADER* header, const char* payload);

bool MakePacket_RES_LOGIN(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload, session_id login_req_id);
void MakePacket_RES_ROOM_LIST(PACKET_HEADER* header, ProtocolBuffer& payload);
bool MakePacket_RES_ROOM_CREATE(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload_buf);
bool MakePacket_RES_ROOM_ENTER(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload, session_id id);
void MakePacket_RES_CHAT(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload, session_id id);
void MakePacket_RES_ROOM_LEAVE(PACKET_HEADER* header, ProtocolBuffer& payload, session_id id);
void MakePacket_RES_ROOM_DELETE(PACKET_HEADER* header, ProtocolBuffer& payload, room_no no);
void MakePacket_RES_USER_ENTER(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& enter_session_data);

// CONTENT
void Update_Logic();