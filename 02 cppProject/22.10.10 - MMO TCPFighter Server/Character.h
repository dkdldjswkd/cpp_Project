#pragma once
#include "Session.h"
#include <unordered_map>
#include "Sector.h"

#define INVALID_SECTOR {-2,-2}

enum class ACTION : BYTE {
	MOVE_LL = 0,
	MOVE_LU = 1,
	MOVE_UU = 2,

	MOVE_RU = 3, // 싱크 뜸
	MOVE_RR = 4,
	MOVE_RD = 5, // 싱크 뜸

	MOVE_DD = 6,
	MOVE_LD = 7, // 싱크 뜸

	STOP = 8,
};

enum class DIR : BYTE {
	DIR_LL = 0,
	DIR_RR = 4,
};

struct Character { // 오브젝트 풀 대상 (기본 생성자 허용, Clear() 필요)
public:
	Character();
	~Character();

public:
	Session* p_session;
	SESSION_ID session_id;

	ACTION action;
	DIR dir;

	int x;
	int y;

	SectorPos cur_sector;
	SectorPos old_sector;

	char hp;

public:
	void Clear(); // 생성자에 준하는 작업
	void Set(Session* p_session, short x, short y, char hp = 100);
};