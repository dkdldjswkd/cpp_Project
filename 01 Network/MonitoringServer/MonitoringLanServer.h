#pragma once
#include <unordered_map>
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/RecursiveLock.h"
#include "../../00 lib_jy/LFObjectPool.h"
#include "ServerSession.h"

class MonitoringLanServer : public NetServer {
public:
	MonitoringLanServer(const char* systemFile, const char* server, NetServer* localServer);
	~MonitoringLanServer();

private:
	// Lib callback (NetServer Override)
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }
	void OnClientJoin(SESSION_ID session_id);
	void OnClientLeave(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);

private:
	LFObjectPool<ServerSession> serverSessionPool;
	std::unordered_map<INT64, ServerSession*> serverSessionMap;
	RecursiveLock serverSessionMapLock;
	NetServer* localServer;
};