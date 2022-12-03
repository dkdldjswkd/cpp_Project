#pragma once
#include "../LanServer.h"

class EchoServer : public LanServer {
private:
	void OnRecv(SESSION_ID session_id, ProtocolBuffer* cs_contentsPacket);
};