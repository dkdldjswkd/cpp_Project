#pragma once
#include "User.h"
#include "Sector.h"

#define ID_LEN				20
#define NICKNAME_LEN		20
typedef INT64 ACCOUNT_NO;

struct Player  : public User {
public:
	Player() ;
	~Player();

private:
	static struct SectorAround {
		int count;
		Sector around[9];
	};

private:
	void Set_SectorAround();

public:
	// Sector
	Sector sectorPos;
	SectorAround sectorAround;

public:
	ACCOUNT_NO accountNo;
	WCHAR id[ID_LEN];
	WCHAR nickname[NICKNAME_LEN];

public:
	void Set(SessionId session_id);

public:
	void Set_Sector(Sector sectorPos);
};