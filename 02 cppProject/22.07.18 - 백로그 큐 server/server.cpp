#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
using namespace std;

#define SERVER_IP   INADDR_ANY
#define SERVER_PORT 7000

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

    int backlog_queue_size = 0;
    if (listen(listen_socket, SOMAXCONN_HINT(65535)) == SOCKET_ERROR) {
        cout << "listen Error" << endl;
        cout << "WSAGetLastError() : " << WSAGetLastError() << endl;
        return 0;
    }

    while (1) {
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}