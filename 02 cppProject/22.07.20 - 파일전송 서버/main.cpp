#pragma warning(disable : 4996)

#pragma comment (lib, "ws2_32.lib")
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
using namespace std;

#define SERVER_IP   INADDR_ANY
#define SERVER_PORT 7000

#define BUF_SIZE 1000
#define KEY 0x11223344

struct st_PACKET_HEADER
{
    DWORD dwPacketCode; // 0x11223344 우리의 패킷확인 고정값

    WCHAR szName[32]; // 본인이름, 유니코드 NULL 문자 끝
    WCHAR szFileName[128]; // 파일이름, 유니코드 NULL 문자 끝
    int iFileSize;
};

char file_buf[500 * 1024] = { 0, };


int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    SOCKET listen_socket;
    SOCKADDR_IN server_addr;

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET) {
        cout << "socket() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cout << "bind() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }
    cout << "listen 성공 accept 대기중" << endl;


    SOCKADDR_IN client_addr;
    int client_addr_size = sizeof(client_addr);

    memset(&client_addr, 0, sizeof(client_addr));
    auto client_socket = accept(listen_socket, (SOCKADDR*)&client_addr, &client_addr_size);
    if (client_socket == INVALID_SOCKET) {
        cout << "accept() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }
    cout << "accept 성공 !!" << endl;

    char recv_buf[BUF_SIZE] = {0,};

    // 최초 헤더 리시브
    int recv_size = recv(client_socket, recv_buf, BUF_SIZE, NULL);
    if (recv_size == SOCKET_ERROR) {
        cout << "recv() Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    // 최초 헤더 판단
    if (KEY == *(DWORD*)recv_buf) {
        cout << "헤더 정상 도착! 파일 데이터 recv!!" << endl;
    }
    else {
        cout << "헤더 데이터가 아닙니다. 종료합니다." << endl;
        return 0;
    }

    // 파일 데이터를 계속 리시브함
    int total_recv_size = 0;
    int file_offset = 0;

    for (int i = 1;; i++) {
        recv_size = recv(client_socket, file_buf + total_recv_size, BUF_SIZE, NULL);
        total_recv_size += recv_size;

        cout << i << "번째 recv size : " << recv_size << endl;

        if (recv_size == 0){
            FILE* wf;
            fopen_s(&wf, "fffff.jpg", "wb");
            if (wf == 0) {
                cout << "wf == 0" << endl;
                return 0;
            }

            fwrite(file_buf, total_recv_size, 1, wf);
            cout << "파일 출력!! 파일 크기 : " << total_recv_size << "(byte)" << endl;

            return 0;
        }
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}