#include "Sector.h"

// true ¹ÝÈ¯ ½Ã INVALID
bool Sector::IsInvalid() {
	if (0 > x || 0 > y)
		return true;
	if (SECTOR_MAX_X <= x || SECTOR_MAX_Y <= y)
		return true;
	return false;
}

bool Sector::operator==(const Sector& other) {
	if (x != other.x)
		return false;
	if (y != other.y)
		return false;
	return true;
}

bool Sector::operator!=(const Sector& other) {
	if (x != other.x)
		return true;
	if (y != other.y)
		return true;
	return false;
}