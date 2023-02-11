#pragma once
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "NetworkLib\LFObjectPool.h"
#include "NetworkLib\LFQueue.h"
#include "NetworkLib\NetworkLib.h"

struct Player {
public:
	Player() {}
	~Player() {}

public:
	SESSION_ID session_id = INVALID_SESSION_ID;
	bool is_connect = false;
	bool is_login = false;

public:
	WCHAR id[ID_LEN];
	WCHAR nickname[NICKNAME_LEN];
	char sessionKey[64]; // 인증토큰

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
		is_connect = false;
		is_login = false;
	}
};

class LoginServer : public NetworkLib {
public:
	LoginServer();
	~LoginServer();

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
};