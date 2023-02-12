#include "stdafx.h"
#include "Define.h"
#include "../../J_LIB/RingBuffer/RingBuffer.h"
#pragma comment(lib, "../../J_LIB/RingBuffer/RingBuffer.lib")

using namespace std;

SOCKET sock;
SOCKADDR_IN server_addr;

char GetRandomCharacter() {
	return 'a' + (rand() % 26);
}

void GetRandomString(char* buf, int len) {
	buf[0] = 0;
	auto max_len = (rand() % len) + 1;
	printf("%d \n", max_len);
	for (int i = 0; i < max_len; i++) {
#pragma warning(suppress : 4996)
		sprintf(buf + i, "%c", GetRandomCharacter());
	}
	buf[max_len] = 0;
}

int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		throw;

	server_addr.sin_family = AF_INET;
#pragma warning(suppress : 4996)
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(SERVER_PORT);

	auto ret_connect = connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
	if (ret_connect == SOCKET_ERROR) 
		throw;

	char str[256];
	char recv_buf[256];
	for (;;) {
		//GetRandomString(str, 255);
		cin >> str;
		auto len = strlen(str);

		auto ret = send(sock, str, len, 0);
		if (ret == SOCKET_ERROR) {
			auto err_no = WSAGetLastError();
			switch (err_no) {
				case 10054: 
					return 0;
			}
		}

		ret = recv(sock, recv_buf, 256, 0);
		if (ret == SOCKET_ERROR) {
			auto err_no = WSAGetLastError();
			switch (err_no) {
			case 10054:
				return 0;
			}
		}
		recv_buf[ret] = 0;

		printf("%s [recv] \n", recv_buf);
	}
}