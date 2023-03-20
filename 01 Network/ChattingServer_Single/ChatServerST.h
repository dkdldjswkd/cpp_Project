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
#include "../../00 lib_jy/ThreadCpuMonitor.h"

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
		SESSION_ID sessionID;
		ACCOUNT_NO accountNo;
		Token token;
	};

	static struct Job {
	public:
		SESSION_ID session_id;
		WORD type;
		PacketBuffer* p_packet;

	public:
		void Set(SESSION_ID session_id, WORD type) {
			this->session_id = session_id;
			this->type = type;
		}
		void Set(SESSION_ID session_id, WORD type, PacketBuffer* p_packet) {
			this->session_id = session_id;
			this->type = type;
			this->p_packet = p_packet;
		}
	};

private:
	// Player
	LFObjectPool<Player> playerPool;
	std::unordered_map<DWORD64, Player*> player_map;						
	std::unordered_set<Player*> sectors_set[SECTOR_MAX_Y][SECTOR_MAX_X];

	// JOB
	LFObjectPoolTLS<Job> jobPool;
	LFQueue<Job*> jobQ; 
	HANDLE updateEvent;
	std::thread updateThread;
	bool updateStop = false;

	// Token
	LFObjectPool<AccountToken> tokenPool;
	LFQueue<AccountToken*> tokenQ;
	cpp_redis::client connectorRedis;
	std::thread authThread;
	HANDLE authEvent;
	bool authStop = false;

	// 모니터링
	int playerCount = 0;
	int updateTPS = 0;

private:
	// Lib callback (NetLib Override)
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);

	// Job
	void UpdateFunc();
	void UpdateStop();
	void JobQueuing(SESSION_ID session_id, WORD type, PacketBuffer* p_packet);
	void JobQueuing(SESSION_ID session_id, WORD type);

	// Token
	void AuthFunc();
	void AuthStop();

	// Send
	void SendSectorAround(Player* p_player, PacketBuffer* send_packet);

public:
	// 서버 종료
	void ServerStop();

	// 모니터링
	DWORD GetPlayerCount();
	DWORD GetPlayerPoolCount();
	DWORD GetJobPoolCount();
	DWORD GetJobQueueCount();
	DWORD GetUpdateTPS();
};

inline DWORD ChatServerST::GetPlayerCount() {
	return playerCount;
}

inline DWORD ChatServerST::GetPlayerPoolCount(){
	return playerPool.GetUseCount();
}

inline DWORD ChatServerST::GetJobPoolCount(){
	return jobPool.GetUseCount();
}

inline DWORD ChatServerST::GetJobQueueCount(){
	return jobQ.GetUseCount();
}

inline DWORD ChatServerST::GetUpdateTPS(){
	auto tmp = updateTPS;
	updateTPS = 0;
	return tmp;
}