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

class LoginServer : public NetServer {
public:
	LoginServer(const char* systemFile, const char* server);
	~LoginServer();

private:
	struct SessionKey {
		char buf[64];
	};

private:
	// User
	RecursiveLock userMapLock;
	std::unordered_map<DWORD64, User*> userMap;
	LFObjectPool<User> userPool;

	// Other Server IP, Port
	char chatServerIP[16];
	char gameServerIP[16];
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
	void OnServerStop() {}

public:
	// 모니터링
	DWORD GetUserCount();
	DWORD GetUserPoolCount();
};

inline DWORD LoginServer::GetUserCount() {
	return userMap.size();
}

inline DWORD LoginServer::GetUserPoolCount() {
	return userPool.GetUseCount();
}