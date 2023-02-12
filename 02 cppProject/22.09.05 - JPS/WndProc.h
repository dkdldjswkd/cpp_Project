#include "stdafx.h"
#include "stRGB.h"
#include "JPS.h"
#include "enums.h"

#define CLASS_NAME "class name"
#define TITLE_NAME "title name"

// 윈도우 사이즈, 시작 위치
#define WINDOW_WIDTH	1650
#define WINDOW_HEIGHT	850
#define CREATE_POS_X	100 
#define CREATE_POS_Y	100

// 그리드 정보
extern unsigned char GRID_SIZE;

// Window
extern HWND h_wnd;
extern MSG msg;
BOOL Init_window();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Render
void RenderGrid(HDC hdc);
void RenderTile(HDC	hdc);
void Draw_Way();
void WM_PAINT_Proc();
void Window_Clear();