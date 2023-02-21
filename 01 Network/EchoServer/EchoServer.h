#pragma once
#include "../NetworkLib/NetworkLib.h"

class EchoServer : public NetworkLib {
public:
	EchoServer(const char* systemFile, const char* server);
	~EchoServer();

public:
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
	void OnError(int errorcode);
};