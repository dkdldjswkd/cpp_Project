#include "Player.h"

Player::Player() {}
Player::~Player() {}

void Player::Set_SectorAround() {
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

void Player::Set(SESSION_ID session_id) {
	this->session_id = session_id;
	sectorPos.x = -2;
	sectorPos.y = -2;
	sectorAround.count = 0;
}

void Player::Set_Sector(Sector sectorPos) {
	this->sectorPos = sectorPos;
	Set_SectorAround();
}