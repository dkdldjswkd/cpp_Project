#include "Sector.h"

// true ��ȯ �� INVALID
bool Sector::IsInvalid() {
	if (0 > x || 0 > y)
		return true;
	if (SECTOR_MAX_X <= x || SECTOR_MAX_Y <= y)
		return true;
	return false;
}