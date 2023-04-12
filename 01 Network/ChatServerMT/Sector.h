#pragma once

// Sector
#define SECTOR_MAX_X		50
#define SECTOR_MAX_Y		50

static struct Sector {
public:
	short x;
	short y;

public:
	bool operator==(const Sector& other);
	bool operator!=(const Sector& other);

public:
	bool IsInvalid();
};