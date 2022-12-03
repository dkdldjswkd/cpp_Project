#include "stdafx.h"
#include "Console.h"

HANDLE  hConsole;

void cs_Init(void)
{
	// 콘솔 핸들 휙득
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO stConsoleCursor;

	// 화면의 커서를 안보이게끔 설정한다.
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize   = 1;
											
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

void cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;

	// 원하는 위치로 커서를 이동시킨다.
	SetConsoleCursorPosition(hConsole, stCoord);
}

// 콘솔 화면 초기화
void cs_ClearScreen(void)
{
	int iCountX, iCountY;
	DWORD dw;

	FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', 100*100, { 0, 0 }, &dw);
}