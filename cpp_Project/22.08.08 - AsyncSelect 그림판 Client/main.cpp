#include "stdafx.h"
#include <windowsx.h>
#pragma warning(disable : 4996)

#include "Util.h"
using namespace std;

#define SERVER_IP	"127.0.0.1"
#define SERVER_PORT	25000

#define BUFSIZE		256
#define WM_SOCKET	(WM_USER + 1)

#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO {
	SOCKETINFO():send_buf(128), recv_buf(128) {}

	SOCKET client_socket = INVALID_SOCKET;
	RingBuffer send_buf;
	RingBuffer recv_buf;
	bool connect_flag = false;
	bool send_flag = true;
};

SOCKETINFO socket_info;

// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct PACKET_DRAW{
	unsigned short Len;
	int		iStartX;
	int		iStartY;
	int		iEndX;
	int		iEndY;
};

SOCKET connect_socket;
HWND h_wnd;
bool drawing;
bool recv_flag;

int prev_x;
int prev_y;
int cur_x;
int cur_y;

char recv_buf[BUFSIZE];
RingBuffer recv_ring_buf(BUFSIZE);

int main(int argc, char* argv[]) {
	int retval;

	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = L"MyWndClass";
	if (!RegisterClass(&wndclass)) return 1;

	// 윈도우 생성
	h_wnd = CreateWindow(L"MyWndClass", L"TCP 서버", WS_OVERLAPPEDWINDOW, 0, 0, 500, 500, NULL, NULL, NULL, NULL);
	if (h_wnd == NULL) return 1;
	ShowWindow(h_wnd, SW_SHOWNORMAL);
	UpdateWindow(h_wnd);

	// 윈속 초기화
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 1;

	// socket ()
	connect_socket = socket(AF_INET, SOCK_STREAM, 0);
	Check_SocketError(connect_socket == INVALID_SOCKET);

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	// listen ()
	retval = connect(connect_socket, (sockaddr*)&server_addr, sizeof(server_addr));
	Check_SocketError(retval == SOCKET_ERROR);

	cout << "connect" << endl;

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(connect_socket, h_wnd, WM_SOCKET, FD_READ | FD_CLOSE);
	Check_SocketError(retval == SOCKET_ERROR);

	// 메시지 루프
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	cout << "close Client" << endl;
	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

// lParam : 상위 16 bit - 오류코드, 하위 16bit - 네트워크 이벤트
void NetworkPorc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	auto error_num = WSAGETSELECTERROR(lParam);

	if (error_num) {
		CRASH();
		return;
	}

	SOCKET client_socket;
	sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ: {
		auto ret_recv = recv(connect_socket, (char*)recv_buf, BUFSIZE, NULL);
		if (ret_recv == 0) {
			goto close;
		}
		else if(ret_recv == SOCKET_ERROR) {
			auto error_num = WSAGetLastError();
			CRASH();
		}

		if (ret_recv != recv_ring_buf.Enqueue(recv_buf, ret_recv)) {
			bool ret = recv_ring_buf.Full();
			CRASH();
		}
		InvalidateRect(hWnd, nullptr, FALSE);
	}
				break;

	case FD_CLOSE:
	close:
		closesocket(connect_socket);
		cout << "FD_CLOSE" << endl;
		break;


	default:
		break;
	}
}

int x_down;
int y_down;

int x_up;
int y_up;


// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_LBUTTONDOWN: {
		cur_x = GET_X_LPARAM(lParam);
		cur_y = GET_Y_LPARAM(lParam);

		drawing = true;

		break;
	}

	case WM_MOUSEMOVE: {
		if (!drawing)
			break;  

		prev_x = cur_x;
		prev_y = cur_y;
		cur_x = GET_X_LPARAM(lParam);
		cur_y = GET_Y_LPARAM(lParam);

		PACKET_DRAW packet_draw;
		packet_draw.iStartX = prev_x;
		packet_draw.iStartY = prev_y;
		packet_draw.iEndX = cur_x;
		packet_draw.iEndY = cur_y;
		packet_draw.Len = 16;

		auto ret = send(connect_socket, (char*)&packet_draw, sizeof(packet_draw), NULL);
		if (SOCKET_ERROR == ret) {
			auto ret = WSAGetLastError();
			CRASH();
		}
		break;
	}

	case WM_LBUTTONUP: {
		drawing = false;

		break;
	}

	case WM_PAINT: {
		printf("WM_PAINT \n");

		HDC hDC;
		PAINTSTRUCT ps;
		hDC = BeginPaint(hWnd, &ps);

		for (;recv_ring_buf.GetUseSize() >= 18;) {
			PACKET_DRAW packet_draw;
			recv_ring_buf.Dequeue((char*)&packet_draw, sizeof(packet_draw));

			MoveToEx(hDC, packet_draw.iStartX, packet_draw.iStartY, NULL);
			LineTo(hDC, packet_draw.iEndX, packet_draw.iEndY);
			printf("(%d, %d) -> (%d, %d) \n", packet_draw.iStartX, packet_draw.iStartY, packet_draw.iEndX, packet_draw.iEndY);
		}
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_SOCKET:
		NetworkPorc(hWnd, uMsg, wParam, lParam);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}