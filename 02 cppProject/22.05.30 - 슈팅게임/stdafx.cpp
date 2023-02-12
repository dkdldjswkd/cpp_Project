#include "stdafx.h"

// x,y 좌표가 콘솔 스크린 안에 포함되는지 체크
bool Pos_Check(int x, int y) {
	if (y < 0 || y >= dfSCREEN_HEIGHT) {
		return false;
	}

	if (x < 0 || x >= dfSCREEN_WIDTH - 1) {
		return false;
	}

	return true;
}