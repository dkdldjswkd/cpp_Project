#pragma once
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include "LFObjectPool.h"
#include "LanServer.h"

#define RANGE_MOVE_TOP		0
#define RANGE_MOVE_LEFT		0
#define RANGE_MOVE_RIGHT	6400
#define RANGE_MOVE_BOTTOM	6400

#define SECTOR_SIZE		200
#define SECTOR_MAX_X	(RANGE_MOVE_RIGHT / SECTOR_SIZE)
#define SECTOR_MAX_Y	(RANGE_MOVE_BOTTOM / SECTOR_SIZE)

#define ID_LEN				20
#define NICKNAME_LEN		20

typedef INT64 ACCOUNT_NO;

static struct Sector {
	int x;
	int y;
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

public:
	WCHAR id[ID_LEN];
	WCHAR nickName[NICKNAME_LEN];
	Sector sectorPos;
	SectorAround sectorAround;

private:
	void Set_SectorAround() {
		sectorAround.count = 0;

		if (sectorPos.x < 0 || sectorPos.y < 0) {
			sectorAround.count = 0;
			return;
		}

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
	void Set(SESSION_ID session_id = INVALID_SESSION_ID, WCHAR* id = nullptr, WCHAR* nickName = nullptr, Sector sectorPos = { -1,-1 }) {
		this->session_id = session_id;
		Set_ID(id);
		Set_Nickname(nickName);
		Set_Sector(sectorPos);
	}
	inline void Set_ID(WCHAR* id) {
		if (id != nullptr) {
#pragma warning(suppress : 4996)
			wcsncpy(this->id, id, ID_LEN);
			this->id[ID_LEN] = 0;
		}
	}
	inline void Set_Nickname(WCHAR* id) {
		if (nickName != nullptr) {
#pragma warning(suppress : 4996)
			wcsncpy(this->nickName, nickName, NICKNAME_LEN);
			this->nickName[NICKNAME_LEN] = 0;
		}
	}
	inline void Set_Sector(Sector sectorPos) {
		this->sectorPos = sectorPos;
		Set_SectorAround();
	}
};

class ChattingServer: public LanServer {
public:
	ChattingServer();
	~ChattingServer();

private:
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	std::thread updateThread;
	std::thread heartbeatThread;

private:
	J_LIB::LFObjectPool<Player> playerPool;
	std::unordered_map<DWORD64, Player*> player_map;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, J_LIB::PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
	void OnError(int errorcode);

private:
	void UpdateFunc();
	void HeartbeatFunc();
	void Disconnect_Player(Player* p_player);
	
private:
	bool ProcPacket(Player* p_player, WORD type, J_LIB::PacketBuffer* cs_contentsPacket);
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket);
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket);
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket);
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_HEARTBEATE(Player* p_player, J_LIB::PacketBuffer* cs_contentsPacket);
};