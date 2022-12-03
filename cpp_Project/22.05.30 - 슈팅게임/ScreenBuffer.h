#pragma once
#include "Console.h"

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

// 스크린 출력 ( 80*24 )
void Buffer_Flip(void);
// 스크린 버퍼 클리어
void Buffer_Clear(void);
// 좌표에 문자 출력
void Sprite_Draw(int iX, int iY, char chSprite);