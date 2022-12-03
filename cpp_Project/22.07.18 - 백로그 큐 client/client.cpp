#pragma warning(disable : 4996)

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
using namespace std;

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 7000

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

    int backlog_queue_size = 0;
    while (1) {
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

            while (1) {

            }

            return 0;
        }

        backlog_queue_size++;
        cout << backlog_queue_size << endl;
    }

    WSACleanup();
    return 0;
}