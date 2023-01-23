#pragma once
#include "LanServer.h"

class EchoServer : public NetworkLib {
public:
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, J_LIB::PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
	void OnError(int errorcode);
};