#include "Player.h"

Player::Player() {}
Player::~Player() {}

void Player::SetSectorAround() {
	sectorAround.count = 0;

	if (true == sectorPos.IsInvalid())
		return;

	int sectorX = sectorPos.x - 1;
	int sectorY = sectorPos.y - 1;
	for (int y = 0; y < 3; y++) {
		if (sectorY + y < 0 || sectorY + y >= SECTOR_MAX_Y)
			continue;

		for (int x = 0; x < 3; x++) {
			if (sectorX + x < 0 || sectorX + x >= SECTOR_MAX_X)
				continue;

			sectorAround.around[sectorAround.count].x = sectorX + x;
			sectorAround.around[sectorAround.count].y = sectorY + y;
			sectorAround.count++;
		}
	}
}

void Player::Set(SessionId sessionId) {
	this->sessionId = sessionId;
	sectorPos.x = -2;
	sectorPos.y = -2;
	sectorAround.count = 0;
}

void Player::SetSector(Sector sectorPos) {
	this->sectorPos = sectorPos;
	SetSectorAround();
}