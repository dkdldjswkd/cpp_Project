#pragma once
#include "Console.h"

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

class BaseObject;

// 스크린 출력 ( 80*24 )
void Buffer_Flip(void);
// 스크린 버퍼 클리어
void Buffer_Clear(void);
// 좌표에 문자 출력
void Sprite_Draw(int iX, int iY, char chSprite);
// 유요한 포지션인지 판단
bool Pos_Check(int x, int y);
// 같은 포지션인지 판단
bool Cmp_Pos(int x, int y, int other_x, int other_y);
//bool Cmp_Pos(const BaseObject& me, const BaseObject& other);

