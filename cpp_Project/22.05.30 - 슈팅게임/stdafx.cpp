#include "stdafx.h"

// x,y ��ǥ�� �ܼ� ��ũ�� �ȿ� ���ԵǴ��� üũ
bool Pos_Check(int x, int y) {
	if (y < 0 || y >= dfSCREEN_HEIGHT) {
		return false;
	}

	if (x < 0 || x >= dfSCREEN_WIDTH - 1) {
		return false;
	}

	return true;
}