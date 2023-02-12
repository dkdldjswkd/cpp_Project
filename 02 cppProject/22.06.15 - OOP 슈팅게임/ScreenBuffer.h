#pragma once
#include "Console.h"

extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

class BaseObject;

// ��ũ�� ��� ( 80*24 )
void Buffer_Flip(void);
// ��ũ�� ���� Ŭ����
void Buffer_Clear(void);
// ��ǥ�� ���� ���
void Sprite_Draw(int iX, int iY, char chSprite);
// ������ ���������� �Ǵ�
bool Pos_Check(int x, int y);
// ���� ���������� �Ǵ�
bool Cmp_Pos(int x, int y, int other_x, int other_y);
//bool Cmp_Pos(const BaseObject& me, const BaseObject& other);

