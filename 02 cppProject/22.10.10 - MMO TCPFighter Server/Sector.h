#pragma once
#include "Protocol.h"
#include "Define.h"



struct Character;

extern std::unordered_set<Character*> sectors[SECTOR_MAX_Y][SECTOR_MAX_X];

struct SectorPos {
public:
	int x;
	int y;

public:
	void operator=(const SectorPos& other);
	bool operator==(const SectorPos& other);
	bool operator!=(const SectorPos& other);
};

struct SectorAround {
	int count;
	SectorPos around[9];
};

struct SectorFunc {
	static void	Insert_Sector(Character* p_character);
	static bool	Erase_Sectors(Character* p_character);
	static bool	Update_SectorPos(Character* p_character);
	static void	Get_SectorAround(int sector_x, int sector_y, SectorAround* p_sectorAround);
	static void	Get_ChangedSectorAround(Character* p_character, SectorAround* p_removeSector, SectorAround* addSector);
};