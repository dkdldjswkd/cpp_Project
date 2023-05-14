#pragma once
#include <cpp_redis/cpp_redis>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "../../00 lib_jy/LFObjectPool.h"
#include "../../00 lib_jy/LFObjectPoolTLS.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../NetworkLib/NetServer.h"
#include "Player.h"

// Login On/Off
#define ON_LOGIN	1

// JOB
#define JOB_TYPE_CLIENT_JOIN			100
#define JOB_TYPE_CLIENT_LEAVE			101
#define JOB_TYPE_CLIENT_LOGIN_SUCCESS	102
#define JOB_TYPE_CLIENT_LOGIN_FAIL		103

class ChatServerST: public NetServer {
public:
	ChatServerST(const char* systemFile, const char* server);
	~ChatServerST();

private:
	static struct Token {
		char buf[64];
	};

	static struct AccountToken {
		SessionId sessionId;
		AccountNo accountNo;
		Token token;
	};

	static struct Job {
	public:
		SessionId sessionId;
		WORD type;
		PacketBuffer* p_packet;

	public:
		void Set(SessionId sessionId, WORD type) {
			this->sessionId = sessionId;
			this->type = type;
		}
		void Set(SessionId sessionId, WORD type, PacketBuffer* p_packet) {
			this->sessionId = sessionId;
			this->type = type;
			this->p_packet = p_packet;
		}
	};

private:
	// Player
	LFObjectPool<Player> playerPool;
	std::unordered_map<DWORD64, Player*> playerMap;						
	std::unordered_set<Player*> sectorSet[SECTOR_MAX_Y][SECTOR_MAX_X];

	// JOB
	LFObjectPoolTLS<Job> jobPool;
	LFQueue<Job*> jobQ; 
	HANDLE updateEvent;
	std::thread updateThread;
	bool updateStop = false;

	// Token
	LFObjectPool<AccountToken> tokenPool;
	LFQueue<AccountToken*> tokenQ;
	cpp_redis::client redisClient;
	std::thread authThread;
	HANDLE authEvent;
	bool authStop = false;

	// 모니터링
	int playerCount = 0;
	int updateCount = 0;
	int updateTPS = 0;

private:
	// Lib callback (NetLib Override)
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }
	void OnClientJoin(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contents_packet);
	void OnClientLeave(SessionId sessionId);
	void OnServerStop();

	// Job
	void UpdateFunc();
	void UpdateStop();
	void JobQueuing(SessionId sessionId, WORD type, PacketBuffer* p_packet);
	void JobQueuing(SessionId sessionId, WORD type);

	// Token
	void AuthFunc();
	void AuthStop();

	// Send
	void SendSectorAround(Player* p_player, PacketBuffer* send_packet);

public:
	// 모니터링
	void UpdateTPS();
	DWORD GetUpdateTPS();
	DWORD GetUserCount();
	DWORD GetUserPoolCount();
	DWORD GetJobPoolCount();
	DWORD GetJobQueueCount();
};

inline void ChatServerST::UpdateTPS() {
	updateTPS = updateCount;
	updateCount = 0;
}

inline DWORD ChatServerST::GetUpdateTPS(){
	return updateTPS;
}

inline DWORD ChatServerST::GetUserCount() {
	return playerCount;
}

inline DWORD ChatServerST::GetUserPoolCount(){
	return playerPool.GetUseCount();
}

inline DWORD ChatServerST::GetJobPoolCount(){
	return jobPool.GetUseCount();
}

inline DWORD ChatServerST::GetJobQueueCount(){
	return jobQ.GetUseCount();
}