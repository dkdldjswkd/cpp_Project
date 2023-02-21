#pragma once
#include "../NetworkLib/NetworkLib.h"

class EchoGameServer : public NetworkLib {
public:
	EchoGameServer(const char* systemFile, const char* server);
	~EchoGameServer();

public:
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
	void OnError(int errorcode);
};