#pragma once
#include <Windows.h>
#include "Sector.h"
#include "../NetworkLib/NetworkLib.h"

// Player
#define ID_LEN				20
#define NICKNAME_LEN		20
typedef INT64 ACCOUNT_NO;

struct Token {
	char buf[64];
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
	inline void Reset();
	inline void Set_Sector(Sector sectorPos);
};

inline void Player::Set_SectorAround() {
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

inline void Player::Set(SESSION_ID session_id) {
	this->session_id = session_id;
	sectorPos.x = -2;
	sectorPos.y = -2;
	sectorAround.count = 0;
	is_login = true;
}

inline void Player::Set_Sector(Sector sectorPos) {
	this->sectorPos = sectorPos;
	Set_SectorAround();
}

inline void Player::Reset() {
	is_login = false;
}