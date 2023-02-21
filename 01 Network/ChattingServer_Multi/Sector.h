#pragma once

// Sector
#define SECTOR_MAX_X		50
#define SECTOR_MAX_Y		50

static struct Sector {
public:
	int x;
	int y;

public:
	// true ¹ÝÈ¯ ½Ã INVALID
	inline bool CheckInvalid() {
		if (0 > x || 0 > y)
			return true;
		if (SECTOR_MAX_X <= x || SECTOR_MAX_Y <= y)
			return true;
		return false;
	}

	bool operator==(const Sector& other) {
		if (x != other.x)
			return false;
		if (y != other.y)
			return false;
		return true;
	}

	bool operator!=(const Sector& other) {
		if (x != other.x)
			return true;
		if (y != other.y)
			return true;
		return false;
	}
};