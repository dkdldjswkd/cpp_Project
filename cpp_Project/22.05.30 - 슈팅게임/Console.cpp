#include "stdafx.h"
#include "Console.h"

HANDLE  hConsole;

void cs_Init(void)
{
	// �ܼ� �ڵ� �׵�
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO stConsoleCursor;

	// ȭ���� Ŀ���� �Ⱥ��̰Բ� �����Ѵ�.
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize   = 1;
											
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

// �ܼ� ȭ�� �ʱ�ȭ
void cs_ClearScreen(void)
{
	int iCountX, iCountY;
	DWORD dw;

	FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 100*100, { 0, 0 }, &dw);
}