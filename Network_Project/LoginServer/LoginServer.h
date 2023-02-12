#pragma once
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "NetworkLib\LFObjectPool.h"
#include "NetworkLib\LFQueue.h"
#include "NetworkLib\NetworkLib.h"
#include "NetworkLib\DBConnector.h"

struct Player {
public:
	Player() {}
	~Player() {}

public:
	SESSION_ID session_id = INVALID_SESSION_ID;
	INT64	accountNo;
	char	SessionKey[64];

public:
	void Set() {}
	void Reset() {}
};

class LoginServer : public NetworkLib {
public:
	LoginServer();
	~LoginServer();

private:
	J_LIB::LFObjectPool<Player> playerPool;
	std::unordered_map<SESSION_ID, Player*> playerMap;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
};