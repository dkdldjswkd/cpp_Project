#ifndef __CONSOLE__
#define __CONSOLE__

// �ܼ� ���� - �ܼ� �ڵ��׵�, Ŀ�� ����ȭ
void cs_Init(void);
// �ܼ� Ŀ�� �̵�
void cs_MoveCursor(int iPosX, int iPosY);
// �ܼ� ȭ�� �ʱ�ȭ
void cs_ClearScreen(void);

#endif