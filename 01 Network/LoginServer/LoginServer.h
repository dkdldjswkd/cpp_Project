#pragma once
#include <cpp_redis/cpp_redis>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "../NetworkLib/LFObjectPool.h"
#include "../NetworkLib/LFQueue.h"
#include "../NetworkLib/NetworkLib.h"
#include "../DBConnector/DBConnectorTLS.h"
#include "../NetworkLib/RecursiveLock.h"

struct Token {
	char buf[64];
};

struct Player {
public:
	Player() {}
	~Player() {}

public:
	SESSION_ID session_id = INVALID_SESSION_ID;

public:
	void Set(SESSION_ID session_id) {
		this->session_id = session_id;
	}
	void Reset() {}
};

class LoginServer : public NetworkLib {
public:
	LoginServer(const char* systemFile, const char* server);
	~LoginServer();

private:
	J_LIB::LFObjectPool<Player> playerPool;
	RecursiveLock playerMap_lock;
	std::unordered_map<DWORD64, Player*> playerMap;

public:
	// DB
	DBConnectorTLS* p_connecterTLS;
	cpp_redis::client connectorRedis;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);

public:
	// 모니터링
	DWORD Get_playerCount();
	DWORD Get_playerPoolCount();
};

inline DWORD LoginServer::Get_playerCount() {
	return playerMap.size();
}

inline DWORD LoginServer::Get_playerPoolCount() {
	return playerPool.Get_UseCount();
}