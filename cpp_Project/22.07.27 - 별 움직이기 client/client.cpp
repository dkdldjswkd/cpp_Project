//#pragma warning(disable : 4996)

#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

#define SERVER_IP	"127.0.0.1"
#define SERVER_PORT	3000

#define MAX_BUF		16

#define PACKET_GET_ID 0
#define PACKET_CREATE_CHARACTER 1
#define PACKET_REMOVE_CHARACTER 2
#define PACKET_MOVE 3

char screen_buf[24][82]; // 0~80, 0~23

struct packet_get_id {
	int type;
	int id;
	long long zero;
};

struct packet_create_character {
	int type;
	int id;
	unsigned int x, y;
};

struct packet_remove_character {
	int type;
	int id;
	long long zero;
};

struct packet_move{
	int type;
	int id;
	unsigned int x, y;
};

#define INVAILD_ID 9999
int my_id = 9999;

struct Player {
	bool flag;
	int x, y;
};

Player players[100];

HANDLE hConsole;
void cs_Init(void)
{
	// 콘솔 핸들 휙득
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO stConsoleCursor;

	// 화면의 커서를 안보이게끔 설정한다.
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;

	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

void cs_MoveCursor(int iPosX, int iPosY)
{
	COORD stCoord;
	stCoord.X = iPosX;
	stCoord.Y = iPosY;

	// 원하는 위치로 커서를 이동시킨다.
	SetConsoleCursorPosition(hConsole, stCoord);
}

void Render() {
	memset(screen_buf, ' ', 24 * 82);
	for (int y = 0; y < 24; y++) {
		screen_buf[y][81] = '\0';
	}

	for (int i = 0; i < 100; i++) {
		if (players[i].flag) {
			screen_buf[players[i].y][players[i].x] = '*';
		}
	}

	int iCnt;
	for (iCnt = 0; iCnt < 24; iCnt++)
	{
		// 화면을 꽉 차게 출력하고 줄바꿈을 하면 화면이 밀릴 수 있으므로 매 줄 출력마다 좌표를 강제로 이동하여 확실하게 출력한다.
		cs_MoveCursor(0, iCnt);
		printf(screen_buf[iCnt]);
	}
}

int main() {
	cs_Init();

	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    SOCKET connect_socket = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect_socket == INVALID_SOCKET) {
        cout << "socket() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    if (connect(connect_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cout << "connect() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	u_long on = 1;
    if (ioctlsocket(connect_socket, FIONBIO, &on) == SOCKET_ERROR) {
        cout << "ioctlsocket() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

	char recv_buf[MAX_BUF];
	char send_buf[MAX_BUF];
	fd_set recv_set;

	int packet_num = 0;
	for (int i = 0;; i++) {
		FD_ZERO(&recv_set);
		FD_SET(connect_socket, &recv_set);
		timeval t = { 0,0 };

		auto act_sock_no = select(NULL, &recv_set, NULL, NULL, &t);
		if (act_sock_no == SOCKET_ERROR) {
			cout << "select() Error" << endl;
			cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
			return 0;
		}

		// Recv 이벤트
		if (FD_ISSET(connect_socket, &recv_set) != 0) {
			auto recv_ret = recv(connect_socket, recv_buf, MAX_BUF, NULL);
			//cout << "Recv 이벤트 발생" << endl;

			// 패킷 분석 로직 구현해야함
			int packet_type = *((int*)recv_buf);
			switch (packet_type) {
				case PACKET_GET_ID: {
					//cout << "Recv Type : " << PACKET_GET_ID << endl;
					my_id = *((int*)(recv_buf + 4));
					//cout << "my_id : " << my_id << endl;
					break;
				}

				case PACKET_CREATE_CHARACTER: {
					//cout << "Recv Type : " << PACKET_CREATE_CHARACTER << endl;
					auto id = *((int*)(recv_buf + 4));
					auto x = *((int*)(recv_buf + 8));
					auto y = *((int*)(recv_buf + 12));

					//cout << "create id : " << id << endl;
					//cout << "x : " << x << endl;
					//cout << "y : " << y << endl;

					players[id].flag = true;
					players[id].x = x;
					players[id].y = y;

					Render();
					break;
				}

				case PACKET_REMOVE_CHARACTER: {
					//cout << "Recv Type : " << PACKET_REMOVE_CHARACTER << endl;
					auto id = *((int*)(recv_buf + 4));
					//cout << "remove id : " << id << endl;
					players[id].flag = false;

					Render();
					break;
				}

				case PACKET_MOVE: {
					//cout << "Recv Type : " << PACKET_MOVE << endl;
					auto id = *((int*)(recv_buf + 4));
					auto x = *((int*)(recv_buf + 8));
					auto y = *((int*)(recv_buf + 12));

					players[id].flag = true;
					players[id].x = x;
					players[id].y = y;

					//cout << "move id : " << id << endl;
					//cout << "x : " << x << endl;
					//cout << "y : " << y << endl;

					Render();
					break;
				}
			}

			cout << endl;
		}

		// 키 입력 로직, 입력있다면 send
		if (my_id != INVAILD_ID && (i % 5000 == 0)) {

			if (GetAsyncKeyState(VK_UP) != 0) {
				if (players[my_id].y > 0) { --players[my_id].y; }

				((packet_move*)send_buf)->type = PACKET_MOVE;
				((packet_move*)send_buf)->id = my_id;
				((packet_move*)send_buf)->x = players[my_id].x;
				((packet_move*)send_buf)->y = players[my_id].y;

				//cout << "move " << endl;
				//cout << "x : " << players[my_id].x << endl;
				//cout << "y : " << players[my_id].y << endl << endl;
				send(connect_socket, send_buf, MAX_BUF, NULL);
				Render();
			}
			else if (GetAsyncKeyState(VK_LEFT) != 0) {
				if (players[my_id].x > 0) { --players[my_id].x; }

				((packet_move*)send_buf)->type = PACKET_MOVE;
				((packet_move*)send_buf)->id = my_id;
				((packet_move*)send_buf)->x = players[my_id].x;
				((packet_move*)send_buf)->y = players[my_id].y;

				//cout << "move" << endl;
				//cout << "x : " << players[my_id].x << endl;
				//cout << "y : " << players[my_id].y << endl << endl;
				send(connect_socket, send_buf, MAX_BUF, NULL);
				Render();
			}
			else if (GetAsyncKeyState(VK_RIGHT) != 0) {
				if (players[my_id].x < 80) { ++players[my_id].x; }

				((packet_move*)send_buf)->type = PACKET_MOVE;
				((packet_move*)send_buf)->id = my_id;
				((packet_move*)send_buf)->x = players[my_id].x;
				((packet_move*)send_buf)->y = players[my_id].y;

				//cout << "move" << endl;
				//cout << "x : " << players[my_id].x << endl;
				//cout << "y : " << players[my_id].y << endl << endl;
				send(connect_socket, send_buf, MAX_BUF, NULL);
				Render();
			}
			else if (GetAsyncKeyState(VK_DOWN) != 0) {
				if (players[my_id].y < 23) { ++players[my_id].y; }

				((packet_move*)send_buf)->type = PACKET_MOVE;
				((packet_move*)send_buf)->id = my_id;
				((packet_move*)send_buf)->x = players[my_id].x;
				((packet_move*)send_buf)->y = players[my_id].y;

				//cout << "move" << endl;
				//cout << "x : " << players[my_id].x << endl;
				//cout << "y : " << players[my_id].y << endl << endl;
				send(connect_socket, send_buf, MAX_BUF, NULL);
				Render();
			}
		}

	}
}