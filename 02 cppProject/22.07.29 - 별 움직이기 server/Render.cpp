#include "stdafx.h"
#include "Render.h"
#include "Server.h"
#include "list"
#include "Actor.h"
using namespace std;

HANDLE hConsole;

char screen_buf[24][82]; // 0~80, 0~23

void cs_Init(void)
{
	// �ܼ� �ڵ� �׵�
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO stConsoleCursor;

	// ȭ���� Ŀ���� �Ⱥ��̰Բ� �����Ѵ�.
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;

	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

void cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;

	// ���ϴ� ��ġ�� Ŀ���� �̵���Ų��.
	SetConsoleCursorPosition(hConsole, stCoord);
}

void Render() {
	memset(screen_buf, ' ', 24 * 82);
	for (int y = 0; y < 24; y++) {
		screen_buf[y][81] = '\0';
	}

	list<Player>::iterator iter = StarServer::Get_Inst().players.begin();
	for (; iter != StarServer::Get_Inst().players.end(); iter++) {
		screen_buf[iter->y][iter->x] = '*';
	}

	int iCnt;
	for (iCnt = 0; iCnt < 24; iCnt++)
	{
		// ȭ���� �� ���� ����ϰ� �ٹٲ��� �ϸ� ȭ���� �и� �� �����Ƿ� �� �� ��¸��� ��ǥ�� ������ �̵��Ͽ� Ȯ���ϰ� ����Ѵ�.
		cs_MoveCursor(0, iCnt);
		printf(screen_buf[iCnt]);
	}
}