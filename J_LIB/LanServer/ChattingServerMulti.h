#pragma once
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "LFObjectPool.h"
#include "LFQueue.h"
#include "LanServer.h"

#define SECTOR_MAX_X		50
#define SECTOR_MAX_Y		50

#define ID_LEN				20
#define NICKNAME_LEN		20

#define JOB_TYPE_CLIENT_JOIN  100
#define JOB_TYPE_CLIENT_LEAVE 101

typedef INT64 ACCOUNT_NO;

static struct Sector {
public:
	int x;
	int y;

public:
	// true 반환 시 INVALID
	inline bool CheckInvalid() {
		if (0 > x || 0 > y)
			return true;
		if (SECTOR_MAX_X <= x || SECTOR_MAX_Y <= y)
			return true;
		return false;
	}

	bool operator==(const Sector& other) {
		if (x != other.x)
			return false;
		if (y != other.y)
			return false;
		return true;
	}

	bool operator!=(const Sector& other) {
		if (x != other.x)
			return true;
		if (y != other.y)
			return true;
		return false;
	}
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
	bool is_connect = false;
	bool is_login = false;

public:
	WCHAR id[ID_LEN];
	WCHAR nickname[NICKNAME_LEN];
	char sessionKey[64]; // 인증토큰

public:
	Sector sectorPos;
	SectorAround sectorAround;

private:
	void Set_SectorAround() {
		sectorAround.count = 0;

		if (sectorPos.CheckInvalid())
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

public:
	inline void Set_Connect(SESSION_ID session_id) {
		this->session_id = session_id;
		is_connect = true;
		sectorPos.x = -2;
		sectorPos.y = -2;
		sectorAround.count = 0;
	}
	inline void Set_Login() {
		is_login = true;
	}
	inline void Set_ID(WCHAR* id) {
			#pragma warning(suppress : 4996)
			wcsncpy(this->id, id, ID_LEN);
			this->id[ID_LEN] = 0;
	}
	inline void Set_Nickname(WCHAR* nickname) {
			#pragma warning(suppress : 4996)
			wcsncpy(this->nickname, nickname, NICKNAME_LEN);
			this->nickname[NICKNAME_LEN] = 0;
	}
	inline void Set_SessionKey(char* key) {
			#pragma warning(suppress : 4996)
			strncpy(sessionKey, key, 64);
	}
	inline void Set_Sector(Sector sectorPos) {
		this->sectorPos = sectorPos;
		Set_SectorAround();
	}
	inline void Reset() {
		// 세션에 대한 경합이 있다면 일괄처리 되어야함
		is_connect = false;
		is_login = false;
	}
};

class ChattingServerMulti : public NetworkLib {
public:
	ChattingServerMulti();
	~ChattingServerMulti();

private:
	static struct Job {
	public:
		SESSION_ID session_id;
		WORD type;
		J_LIB::PacketBuffer* p_packet;

	public:
		void Set(SESSION_ID session_id, WORD type, J_LIB::PacketBuffer* p_packet = nullptr) {
			this->session_id = session_id;
			this->type = type;
			this->p_packet = p_packet;
		}
	};

private:
	// Player
	J_LIB::LFObjectPool<Player> playerPool;
	SRWLOCK playerMap_lock;
	std::unordered_map<DWORD64, Player*> player_map;			
	SRWLOCK sector_lock[SECTOR_MAX_Y][SECTOR_MAX_X];
	std::unordered_set<Player*> sectors_set[SECTOR_MAX_Y][SECTOR_MAX_X];

private:
	// JOB
	J_LIB::LFObjectPool<Job> jobPool;
	LFQueue<Job*> jobQ; 
	HANDLE updateEvent;
	std::thread updateThread;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, J_LIB::PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
	void OnError(int errorcode);

private:
	void Disconnect_Player(Player* p_player);

private:
	void SendSectorAround(Player* p_player, J_LIB::PacketBuffer* send_packet);
	void SendSector(J_LIB::PacketBuffer* send_packet, Sector sector);

private:
	// JOB 처리
	bool ProcPacket(SESSION_ID session_id, WORD type, J_LIB::PacketBuffer* cs_contentsPacket);
	// 패킷(JOB) 처리
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN		(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket);	// 1
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE	(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket);	// 3
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE		(SESSION_ID session_id, J_LIB::PacketBuffer* cs_contentsPacket);	// 5
};