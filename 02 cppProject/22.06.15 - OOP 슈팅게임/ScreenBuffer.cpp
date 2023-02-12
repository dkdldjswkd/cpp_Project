#include "stdafx.h"
#include "Actor.h"
#include "ScreenBuffer.h"

char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

void Buffer_Flip(void) {
	int iCnt;
	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		// ȭ���� �� ���� ����ϰ� �ٹٲ��� �ϸ� ȭ���� �и� �� �����Ƿ� �� �� ��¸��� ��ǥ�� ������ �̵��Ͽ� Ȯ���ϰ� ����Ѵ�.
		cs_MoveCursor(0, iCnt);
		printf(szScreenBuffer[iCnt]);
	}
}

void Buffer_Clear(void) {
	int iCnt;
	memset(szScreenBuffer, ' ', dfSCREEN_WIDTH * dfSCREEN_HEIGHT);

	for (iCnt = 0; iCnt < dfSCREEN_HEIGHT; iCnt++)
	{
		szScreenBuffer[iCnt][dfSCREEN_WIDTH - 1] = '\0';
	}
}

void Sprite_Draw(int iX, int iY, char chSprite) {
	if (iX < 0 || iY < 0 || iX >= dfSCREEN_WIDTH - 1 || iY >= dfSCREEN_HEIGHT)
		return;

	szScreenBuffer[iY][iX] = chSprite;
}

bool Pos_Check(int x, int y) {
	if (y < 0 || y >= dfSCREEN_HEIGHT) {
		return false;
	}
	else if (x < 0 || x >= dfSCREEN_WIDTH - 1) {
		return false;
	}

	return true;
}

bool Cmp_Pos(int x, int y, int other_x, int other_y)
{
	if (x == other_x && y == other_y)
		return true;
	return false;
}

bool Cmp_Pos(const BaseObject& me, const BaseObject& other)
{
	return false;
}
