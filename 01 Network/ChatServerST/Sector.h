#pragma once

// Sector
#define SECTOR_MAX_X		50
#define SECTOR_MAX_Y		50

struct Sector {
public:
	short x;
	short y;

public:
	bool Is_Invalid();
};