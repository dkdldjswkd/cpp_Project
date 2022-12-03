#pragma once

extern HANDLE hConsole;
extern char screen_buf[24][82]; // 0~80, 0~23

void cs_Init(void);
void cs_MoveCursor(int iPosX, int iPosY);
void Render();