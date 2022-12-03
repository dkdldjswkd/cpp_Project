#include "Pos.h"

Pos::Pos() {
}

Pos::Pos(short x, short y) :x(x), y(y) {
}

bool Pos::operator==(const Pos& other) const {
	if (x == other.x && y == other.y) {
		return true;
	}
	return false;
}

bool Pos::operator!=(const Pos& other) const {
	if (x != other.x || y != other.y) {
		return true;
	}
	return false;
}