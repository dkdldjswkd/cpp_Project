#pragma once

#include <WinSock2.h>
#include "RingBuffer.h"

#define SERVER_IP	INADDR_ANY
#define SERVER_PORT	25000
#define INVALID_ID ~0
#define MAX_USER 128
#define BUF_SIZE 256

#define WM_SOCKET	(WM_USER + 1)

#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)

struct SOCKETINFO {
	SOCKETINFO(SOCKET s, int id);
	~SOCKETINFO();

	SOCKET client_socket = INVALID_SOCKET;
	int id = INVALID_ID;
	RingBuffer send_buf;
	bool remove_flag = false;
};

struct PACKET_DRAW {
	unsigned short Len;
	int		iStartX;
	int		iStartY;
	int		iEndX;
	int		iEndY;
};

extern std::unordered_map<SOCKET, SOCKETINFO*> socket_infos;
extern SOCKET listen_sock;
bool Init_Server();
void Run_Server();
WPARAM Close_Server();
int Get_Session_No();

extern HWND h_wnd;
extern WNDCLASS wndclass;
extern MSG msg;
void Set_Window();

// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void NetworkPorc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);