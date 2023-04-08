#pragma once
#include <cpp_redis/cpp_redis>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "User.h"
#include "../../00 lib_jy/LFObjectPool.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../../00 lib_jy/RecursiveLock.h"
#include "../NetworkLib/NetServer.h"
#include "../DBConnector/DBConnectorTLS.h"

struct Token {
	char buf[64];
};

class LoginServer : public NetServer {
public:
	LoginServer(const char* systemFile, const char* server);
	~LoginServer();

private:
	// User
	RecursiveLock UserMapLock;
	std::unordered_map<DWORD64, User*> UserMap;
	LFObjectPool<User> UserPool;

	// Other Server IP, Port
	char chatServerIP[20];
	char gameServerIP[20];
	DWORD ChatServerPort;
	DWORD gameServerPort;

public:
	// DB
	DBConnectorTLS* p_connecterTLS;
	cpp_redis::client connectorRedis;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contentsPacket);
	void OnClientLeave(SessionId sessionId);

public:
	// ����͸�
	DWORD GetPlayerCount();
	DWORD GetPlayerPoolCount();
};

inline DWORD LoginServer::GetPlayerCount() {
	return UserMap.size();
}

inline DWORD LoginServer::GetPlayerPoolCount() {
	return UserPool.GetUseCount();
}