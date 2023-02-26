#pragma once
#include <cpp_redis/cpp_redis>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "../../00 lib_jy/LFObjectPool.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../NetworkLib/NetServer.h"
#include "Player.h"

// Login On/Off
#define ON_LOGIN	0

// JOB
#define JOB_TYPE_CLIENT_JOIN			100
#define JOB_TYPE_CLIENT_LEAVE			101
#define JOB_TYPE_CLIENT_LOGIN_SUCCESS	102
#define JOB_TYPE_CLIENT_LOGIN_FAIL		103


class ChattingServer_Single: public NetServer {
public:
	ChattingServer_Single(const char* systemFile, const char* server);
	~ChattingServer_Single();

private:
	static struct Token {
		char buf[64];
	};

	static struct AccountToken {
		SESSION_ID sessionID;
		ACCOUNT_NO accountNo;
		Token token;

	public:
		void Set() {
			ZeroMemory(this, sizeof(AccountToken));
		}
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
	J_LIB::LFObjectPool<Player> playerPool;
	std::unordered_map<DWORD64, Player*> player_map;						
	std::unordered_set<Player*> sectors_set[SECTOR_MAX_Y][SECTOR_MAX_X];

private:
	// JOB
	J_LIB::LFObjectPool<Job> jobPool;
	LFQueue<Job*> jobQ; 
	HANDLE updateEvent;
	std::thread updateThread;

private:
	// DB
	J_LIB::LFObjectPool<AccountToken> tokenPool;
	LFQueue<AccountToken*> tokenQ;
	cpp_redis::client connectorRedis;
	std::thread tokenThread;
	HANDLE tokenEvent;

private:
	// Lib callback (NetLib Override)
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);

private:
	void UpdateFunc();
	void TokenAuthFunc();
	void JobQueuing(SESSION_ID session_id, WORD type, PacketBuffer* p_packet);
	void JobQueuing(SESSION_ID session_id, WORD type);

private:
	void SendSectorAround(Player* p_player, PacketBuffer* send_packet);
	void SendSector(PacketBuffer* send_packet, Sector sector);

private:
	// JOB ó��
	void ProcJob(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket);
	// ��Ŷ(JOB) ó��
	void ProcJob_en_PACKET_CS_CHAT_REQ_LOGIN		(SESSION_ID session_id, PacketBuffer* cs_contentsPacket);	// 1
	void ProcJob_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE	(SESSION_ID session_id, PacketBuffer* cs_contentsPacket);	// 3
	bool ProcJob_en_PACKET_CS_CHAT_REQ_MESSAGE		(SESSION_ID session_id, PacketBuffer* cs_contentsPacket);	// 5
	// OnFunc(JOB) ó��
	void ProcJob_ClientJoin(SESSION_ID session_id);  // 100
	void ProcJob_ClientLeave(SESSION_ID session_id); // 101
	void ProcJob_ClientLoginSuccess(SESSION_ID session_id); // 102
	void ProcJob_ClientLoginFail(SESSION_ID session_id); // 103
private:
	// ����͸�
	int playerCount = 0;
	int updateTPS = 0;

public:
	// ����͸�
	DWORD Get_playerCount();
	DWORD Get_playerPoolCount();
	DWORD Get_JobPoolCount();
	DWORD Get_JobQueueCount();
	DWORD Get_updateTPS();
};

inline DWORD ChattingServer_Single::Get_playerCount() {
	return playerCount;
}

inline DWORD ChattingServer_Single::Get_playerPoolCount(){
	return playerPool.Get_UseCount();
}

inline DWORD ChattingServer_Single::Get_JobPoolCount(){
	return jobPool.Get_UseCount();
}

inline DWORD ChattingServer_Single::Get_JobQueueCount(){
	return jobQ.GetUseCount();
}

inline DWORD ChattingServer_Single::Get_updateTPS(){
	auto tmp = updateTPS;
	updateTPS = 0;
	return tmp;
}