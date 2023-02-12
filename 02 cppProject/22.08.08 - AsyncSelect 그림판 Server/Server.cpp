#include "stdafx.h"
#pragma warning(disable : 4996)
#include "Server.h"
#include "Util.h"
using namespace std;

SOCKETINFO::SOCKETINFO(SOCKET s, int id) :send_buf(BUF_SIZE), client_socket(s), id(id) {}

SOCKETINFO::~SOCKETINFO() {
	closesocket(client_socket);
}

std::unordered_map<SOCKET, SOCKETINFO*> socket_infos;
SOCKET listen_sock;
SOCKET sockets[MAX_USER]; // socket_infos의 key값을 모아둠
queue<SOCKET> remove_queue;

bool Init_Server() {
	Set_Window();
	for (auto& v : sockets) {
		v = INVALID_SOCKET;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	auto ret_error = Check_SocketError(listen_sock == INVALID_SOCKET);

	// addr setting
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	// bind()
	ret_error = Check_SocketError(SOCKET_ERROR == bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)));

	// listen ()
	ret_error = Check_SocketError(SOCKET_ERROR == listen(listen_sock, SOMAXCONN));

	// WSAAsyncSelect()
	ret_error = Check_SocketError(SOCKET_ERROR == WSAAsyncSelect(listen_sock, h_wnd, WM_SOCKET, FD_ACCEPT));

	printf("Init Server \n");
	return true;
}

void Run_Server() {
	printf("Run Server \n");

	// 메시지 루프
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

WPARAM Close_Server() {
	printf("Close Server \n");

	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

int Get_Session_No() {
	static int id = 0;
	return id++;
}

HWND h_wnd;
WNDCLASS wndclass;
MSG msg;
void Set_Window() {
	// 윈도우 클래스 등록
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
	if (!RegisterClass(&wndclass)) {
		cout << "!RegisterClass(&wndclass)" << endl;
		exit(1);
	}

	// 윈도우 생성
	h_wnd = CreateWindow(L"MyWndClass", L"TCP 서버", WS_OVERLAPPEDWINDOW, 0, 0, 500, 500, NULL, NULL, NULL, NULL);
	if (h_wnd == NULL) {
		cout << "hWnd == NULL" << endl;
		exit(1);
	}
	//ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(h_wnd);
}

// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {

	case WM_PAINT: {
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


// lParam : 상위 16 bit - 오류코드, 하위 16bit - 네트워크 이벤트
void NetworkPorc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	auto error_num = WSAGETSELECTERROR(lParam);

	if (error_num) {
		switch (error_num)
		{
		case WSAECONNABORTED:
			break;

		default:
			CRASH();
			break;
		}
	}

	SOCKET client_socket;
	sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT: {
		client_socket = accept(wParam, (sockaddr*)&client_socket, &addr_len);
		Check_SocketError(INVALID_SOCKET == client_socket);
		socket_infos.insert({ client_socket, new SOCKETINFO(client_socket,  Get_Session_No()) });
		Check_SocketError(SOCKET_ERROR == WSAAsyncSelect(client_socket, hWnd, WM_SOCKET, FD_WRITE | FD_READ | FD_CLOSE));

		// 소켓 리스트에 등록
		for (auto& v : sockets) {
			if (v == INVALID_SOCKET) {
				v = client_socket;
				break;
			}
		}

	}
	printf("[SOCKET : %d, ID : %d] ACCEPT EVENT CLEAR / \n", (int)socket_infos[client_socket]->client_socket, socket_infos[client_socket]->id);
	break;

	case FD_READ: {
		SOCKETINFO* socket_info = socket_infos[wParam];

		char buf[BUF_SIZE];
		auto ret_recv = recv(*(SOCKET*)socket_info, buf, BUF_SIZE, NULL);
		if (ret_recv == 0) {
			remove_queue.push(wParam);
			goto close;
		}
		else {
			Check_SocketError(ret_recv == SOCKET_ERROR);
		}

		// recv 데이터 send buf에 Enqueue // 브로트 캐스팅 하기 위함, 버퍼가 가득 찬 애들 끊어버리기?
		for (const auto& v : sockets) {
			if (v != INVALID_SOCKET) {
				const auto enqueue_size = socket_infos[v]->send_buf.Enqueue(buf, ret_recv);
				if (ret_recv != enqueue_size) {
					if (socket_infos[v]->send_buf.Full()) {
						remove_queue.push(v);
						socket_infos[v]->remove_flag = true;
					}
					else {
						// 이건 무슨경우???
						CRASH();
					}
				}
			}
		}
	}
	printf("[SOCKET : %d, ID : %d] RECV EVENT OCCUR / \n", (int)wParam, socket_infos[wParam]->id);
	// FD_READ -> FD_WRITE 연계

	case FD_WRITE:
		for (const auto& v : sockets) {
			if (v == INVALID_SOCKET)
				continue;

			const int use_size = socket_infos[v]->send_buf.GetUseSize();
			if (use_size < sizeof(PACKET_DRAW))
				continue;

			if (socket_infos[v]->remove_flag)
				continue;

			SOCKETINFO* socket_info = socket_infos[v];

			const int send_size = (use_size / sizeof(PACKET_DRAW)) * sizeof(PACKET_DRAW);

			char buf[BUF_SIZE];
			auto peek_len = socket_info->send_buf.Peek(buf, send_size);
			auto ret_send = send(*(SOCKET*)socket_info, buf, peek_len, NULL);
			auto error_num = Check_SocketError(ret_send == SOCKET_ERROR);
			socket_info->send_buf.MoveFront(ret_send);

			printf("[SOCKET : %d, ID : %d] SEND EVENT CLEAR / send size : %d \n", (int)v, socket_info->id, ret_send);
		}
		printf("SEND CLEAR \n");
		break;

	case FD_CLOSE: {
		remove_queue.push(wParam);

	close:

		for (; !remove_queue.empty(); remove_queue.pop()) {
			SOCKET key = remove_queue.front();

			printf("[SOCKET : %d, ID : %d] CLOSE EVENT OCCUR / \n", (int)key, socket_infos[key]->id);
			delete socket_infos[key];
			socket_infos.erase(wParam);
		}
	}
	 break;

	default: {

	}
	 break;

	}

}