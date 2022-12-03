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
	// 콘솔 핸들 휙득
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO stConsoleCursor;

	// 화면의 커서를 안보이게끔 설정한다.
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;

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
		// 화면을 꽉 차게 출력하고 줄바꿈을 하면 화면이 밀릴 수 있으므로 매 줄 출력마다 좌표를 강제로 이동하여 확실하게 출력한다.
		cs_MoveCursor(0, iCnt);
		printf(screen_buf[iCnt]);
	}
}