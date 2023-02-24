#pragma once
#include <unordered_map>
#include "../NetworkLib/NetServer.h"
#include "../NetworkLib/RecursiveLock.h"
#include "../NetworkLib/LFObjectPool.h"
#include "User.h"

class MonitoringServer : public NetServer {
public:
	MonitoringServer(const char* systemFile, const char* server);
	~MonitoringServer();

private:
	// Lib callback (NetLib Override)
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);

private:
	J_LIB::LFObjectPool<User> userPool;
	std::unordered_map<INT64, User*> userMap;
	RecursiveLock userMapLock;

private:
	char loginKey[32];

private:
	void ProcPacket(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket);
};