#pragma once
#include <cpp_redis/cpp_redis>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../NetworkLib/LFObjectPool.h"
#include "../NetworkLib/LFQueue.h"
#include "../NetworkLib/NetworkLib.h"

// Login On/Off
#define ON_LOGIN	0

// Sector
#define SECTOR_MAX_X		50
#define SECTOR_MAX_Y		50

// Player
#define ID_LEN				20
#define NICKNAME_LEN		20
typedef INT64 ACCOUNT_NO;

// JOB
#define JOB_TYPE_CLIENT_JOIN			100
#define JOB_TYPE_CLIENT_LEAVE			101
#define JOB_TYPE_CLIENT_LOGIN_SUCCESS	102
#define JOB_TYPE_CLIENT_LOGIN_FAIL		103

struct Token {
	char buf[64];
};

struct AccountToken {
	SESSION_ID sessionID;
	ACCOUNT_NO accountNo;
	Token token;

public:
	void Set() {
		ZeroMemory(this, sizeof(AccountToken));
	}
};

struct Sector {
public:
	short x;
	short y;

public:
	bool Is_Invalid();
};

struct Player {
public:
	Player() {}
	~Player() {}

private:
	static struct SectorAround {
		int count;
		Sector around[9];
	};

public:
	SESSION_ID session_id = INVALID_SESSION_ID;
	ACCOUNT_NO accountNo;
	bool is_login = false;

public:
	WCHAR id[ID_LEN];
	WCHAR nickname[NICKNAME_LEN];
	Token token;

public:
	Sector sectorPos;
	SectorAround sectorAround;

private:
	void Set_SectorAround();

public:
	inline void Set(SESSION_ID session_id);
	inline void Set_Sector(Sector sectorPos);
	inline void Reset();
};

class ChattingServer_Single: public NetworkLib {
public:
	ChattingServer_Single(const char* systemFile, const char* server);
	~ChattingServer_Single();

private:
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
	// Player �����̳�
	J_LIB::LFObjectPool<Player> playerPool;
	std::unordered_map<DWORD64, Player*> player_map;						// �����ڿ�
	std::unordered_set<Player*> sectors_set[SECTOR_MAX_Y][SECTOR_MAX_X];	// �����ڿ�

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
	void 
	(int errorcode);

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

//////////////////////////////
// Sector
//////////////////////////////

// true ��ȯ �� INVALID
inline bool Sector::Is_Invalid() {
	if (0 > x || 0 > y)
		return true;
	if (SECTOR_MAX_X <= x || SECTOR_MAX_Y <= y)
		return true;
	return false;
}

//////////////////////////////
// Player
//////////////////////////////

inline void Player::Set_SectorAround() {
	sectorAround.count = 0;

	if (true == sectorPos.Is_Invalid())
		return;

	int sector_x = sectorPos.x - 1;
	int sector_y = sectorPos.y - 1;
	for (int y = 0; y < 3; y++) {
		if (sector_y + y < 0 || sector_y + y >= SECTOR_MAX_Y)
			continue;

		for (int x = 0; x < 3; x++) {
			if (sector_x + x < 0 || sector_x + x >= SECTOR_MAX_X)
				continue;

			sectorAround.around[sectorAround.count].x = sector_x + x;
			sectorAround.around[sectorAround.count].y = sector_y + y;
			sectorAround.count++;
		}
	}
}

inline void Player::Set(SESSION_ID session_id) {
	this->session_id = session_id;
	sectorPos.x = -2;
	sectorPos.y = -2;
	sectorAround.count = 0;
}
inline void Player::Set_Sector(Sector sectorPos) {
	this->sectorPos = sectorPos;
	Set_SectorAround();
}
inline void Player::Reset() {
	is_login = false;
}