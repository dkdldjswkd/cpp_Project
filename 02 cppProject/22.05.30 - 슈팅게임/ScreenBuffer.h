#pragma once
#include "Console.h"

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

// ��ũ�� ��� ( 80*24 )
void Buffer_Flip(void);
// ��ũ�� ���� Ŭ����
void Buffer_Clear(void);
// ��ǥ�� ���� ���
void Sprite_Draw(int iX, int iY, char chSprite);