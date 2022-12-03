#pragma warning(disable : 4996)

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <wchar.h>
using namespace std;

#define SERVER_IP   "106.245.38.107"
#define SERVER_PORT 10010

#define BUF_SIZE 1000

struct st_PACKET_HEADER
{
    DWORD dwPacketCode; // 0x11223344 우리의 패킷확인 고정값

    WCHAR szName[32]; // 본인이름, 유니코드 NULL 문자 끝
    WCHAR szFileName[128]; // 파일이름, 유니코드 NULL 문자 끝
    int iFileSize;
};

char file_buf[500 * 1024];
char check_buf[500 * 1024];
char send_buf[BUF_SIZE];

int main() {
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

	SOCKET connect_socket;

	connect_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (connect_socket == INVALID_SOCKET) {
		cout << "socket() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	if (connect(connect_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		cout << "connect() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}

	// read file
	FILE* rf;
	fopen_s(&rf, "cat.jpg", "rb");
	int file_size;
	if (rf == 0) {
		cout << "rf == 0" << endl;
		return 0;
	}
	else {
		cout << "read file 성공" << endl;

		// 파일 사이즈 구하기
		fseek(rf, 0, SEEK_END);
		file_size = ftell(rf);
		fseek(rf, 0, SEEK_SET);

		fread(file_buf, 1, file_size, rf);
		cout << "file_size : " << file_size << endl;
	}

	//// Test code : fread로 긁어온 데이터가 무결한지
	//{
	//	FILE* wf;
	//	fopen_s(&wf, "test.jpg", "wb");
	//	fwrite(file_buf, 1, file_size, wf);
	//	fclose(wf);
	//}

	// 헤더 패킷 생성
	st_PACKET_HEADER header_packit;
	header_packit.dwPacketCode = 0x11223344;
	wcscpy(header_packit.szName, L"최준영");
	wcscpy(header_packit.szName, L"cat.jpg");
	header_packit.iFileSize = file_size;

	memcpy(send_buf, &header_packit, sizeof(header_packit));

	// 헤더 패킷 send
	int send_size = send(connect_socket, (char*)&header_packit, sizeof(header_packit), NULL);
	if (send_size == SOCKET_ERROR) {
		cout << "send() Error" << endl;
		cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
		return 0;
	}
	else {
		cout << "헤더 send, 헤더 사이즈 : " << send_size << endl;
	}

	// 파일 데이터 send
	int total_send_size = 0;
	int remain_file_size = file_size;
	for (int i = 1;; i++) {

		int len;
		if (remain_file_size < BUF_SIZE) {
			len = remain_file_size;
		}
		else {
			len = BUF_SIZE;
		}

		send_size = send(connect_socket, file_buf + total_send_size, len, NULL);

		// 디버깅 코드 send로 보내는 데이터가 무결한지
		memcpy(check_buf + total_send_size, file_buf + total_send_size, len);

		remain_file_size -= send_size;
		total_send_size += send_size;


		cout << i << "번째 send size : " << send_size << endl;
		cout << "remain_file_size : " << remain_file_size << endl;

		if (remain_file_size <= 0) {
			// 디버깅 코드 send로 보내는 데이터가 무결한지
			FILE* wf;
			fopen_s(&wf, "test.jpg", "wb");
			fwrite(file_buf, 1, total_send_size, wf);
			fclose(wf);

			cout << "파일 데이터 전송 완료" << endl;
			break;
		}
	}

	closesocket(connect_socket);
	WSACleanup();
	return 0;
}