#pragma once
#include "User.h"
#include "Sector.h"

#define ID_LEN				20
#define NICKNAME_LEN		20
typedef INT64 AccountNo;

struct Player : public User {
public:
	Player();
	~Player();

private:
	static struct SectorAround {
		int count;
		Sector around[9];
	};

private:
	void SetSectorAround();

public:
	// Sector
	Sector sectorPos;
	SectorAround sectorAround;

public:
	AccountNo accountNo;
	WCHAR id[ID_LEN];
	WCHAR nickname[NICKNAME_LEN];

public:
	void Set(SessionId sessionId);
	void SetSector(Sector sectorPos);
};