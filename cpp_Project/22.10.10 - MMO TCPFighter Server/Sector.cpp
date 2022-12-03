#include "stdafx.h"
#include "Sector.h"
#include "Character.h"

using namespace std;

//----------------------------------
// 변수
//----------------------------------
unordered_set<Character*> sectors[SECTOR_MAX_Y][SECTOR_MAX_X];

//----------------------------------
// SectorPos
//----------------------------------
void SectorPos::operator=(const SectorPos& other){
    x = other.x;
    y = other.y;
}

bool SectorPos::operator==(const SectorPos& other) {
    return (x == other.x && y == other.y);
}

bool SectorPos::operator!=(const SectorPos& other) {
    return (x != other.x || y != other.y);
}

//----------------------------------
// Sector Func
//----------------------------------
void SectorFunc::Insert_Sector(Character* p_character) {
    sectors[p_character->cur_sector.y][p_character->cur_sector.x].insert(p_character); 
}

// Sectors.Erase (캐릭터 Cur Sector 좌표 기준)
bool SectorFunc::Erase_Sectors(Character* p_character){
    auto iter = sectors[p_character->cur_sector.y][p_character->cur_sector.x].find(p_character);
    if (iter == sectors[p_character->cur_sector.y][p_character->cur_sector.x].end())
        throw;
    
    sectors[p_character->cur_sector.y][p_character->cur_sector.x].erase(iter);
    return true;
}

// Old/Cur SectorPos 최신화
bool SectorFunc::Update_SectorPos(Character* p_character) {
    SectorPos update_pos;
	update_pos.x = p_character->x / SECTOR_SIZE;
	update_pos.y = p_character->y / SECTOR_SIZE;

	// 섹터 변경 X
	if (p_character->cur_sector == update_pos)
		return false;

	// 섹터 변경 O
	Erase_Sectors(p_character);
	p_character->old_sector = p_character->cur_sector;
    p_character->cur_sector = update_pos;
	Insert_Sector(p_character);

    // 디버깅
    if(DEBUF_LEVEL >= 3)
    
        
        ("[Sector Change] id(%d) // old(%d, %d) -> cur(%d, %d) \n", p_character->session_id, p_character->old_sector.x, p_character->old_sector.y, p_character->cur_sector.x, p_character->cur_sector.y);
	return true;
}

void SectorFunc::Get_SectorAround(int sector_x, int sector_y, SectorAround* p_sectorAround){
    sector_x--;
    sector_y--;

    p_sectorAround->count = 0;

    for (int y = 0; y < 3; y++) {
        if (sector_y + y < 0 || sector_y + y >= SECTOR_MAX_Y)
            continue;

        for (int x = 0; x < 3; x++) {
            if (sector_x + x < 0 || sector_x + x >= SECTOR_MAX_X)
                continue;

            p_sectorAround->around[p_sectorAround->count].x = sector_x + x;
            p_sectorAround->around[p_sectorAround->count].y = sector_y + y;
            p_sectorAround->count++;
        }
    }
}

// Get Remove Sector, Add Sector
void SectorFunc::Get_ChangedSectorAround(Character* p_character, SectorAround* p_removeSector, SectorAround* p_addSector){
    SectorAround old_sectorAround, cur_sectorAround;

    old_sectorAround.count = 0;
    cur_sectorAround.count = 0;
    p_removeSector->count = 0;
    p_addSector->count = 0;

    Get_SectorAround(p_character->old_sector.x, p_character->old_sector.y, &old_sectorAround);
    Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &cur_sectorAround);

    // Get Remove Sector (Old Sector Around에는 있지만, Cur Sector Around에는 없는 Sector)
    for (int old = 0; old < old_sectorAround.count; old++) {
        bool find = false;

        for (int cur = 0; cur < cur_sectorAround.count; cur++) {
            if (old_sectorAround.around[old].x == cur_sectorAround.around[cur].x &&
                old_sectorAround.around[old].y == cur_sectorAround.around[cur].y) {
                find = true;
                break;
            }
        }
        // Old 에만 있는 Sector
        if (!find) {
            p_removeSector->around[p_removeSector->count] = old_sectorAround.around[old];
            p_removeSector->count++;
        }
    }

    // Get Add Sector (Cur Sector Around에는 있지만, Old Sector Around에는 없는 Sector)
    for (int cur = 0; cur < cur_sectorAround.count; cur++) {
        bool find = false;

        for (int old = 0; old < old_sectorAround.count; old++) {
            if (cur_sectorAround.around[cur].x == old_sectorAround.around[old].x &&
                cur_sectorAround.around[cur].y == old_sectorAround.around[old].y) {
                find = true;
                break;
            }
        }
        // Cur 에만 있는 Sector
        if (!find) {
            p_addSector->around[p_addSector->count] = cur_sectorAround.around[cur];
            p_addSector->count++;
        }
    }
}