#pragma once

#define CLASS_NAME "ClassName"
#define TITLE_NAME "RedBlack Tree"

extern MSG	msg;
extern HWND h_wnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM w_param, LPARAM l_param);
void Init_Window();