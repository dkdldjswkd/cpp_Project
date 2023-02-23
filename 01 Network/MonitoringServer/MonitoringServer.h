#pragma once
#include "../NetworkLib/NetworkLib.h"
#include <unordered_map>
#include "../NetworkLib/RecursiveLock.h"
#include "../NetworkLib/LFObjectPool.h"
#include "User.h"

class MonitoringServer : public NetworkLib {
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
	std::unordered_map<SESSION_ID, User*> userMap;
	RecursiveLock userMapLock;

private:
	char loginKey[32];

private:
	void ProcPacket(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket);
};