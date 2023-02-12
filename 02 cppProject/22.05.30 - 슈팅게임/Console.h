#ifndef __CONSOLE__
#define __CONSOLE__

// 콘솔 셋팅 - 콘솔 핸들휙득, 커서 투명화
void cs_Init(void);
// 콘솔 커서 이동
void cs_MoveCursor(int iPosX, int iPosY);
// 콘솔 화면 초기화
void cs_ClearScreen(void);

#endif