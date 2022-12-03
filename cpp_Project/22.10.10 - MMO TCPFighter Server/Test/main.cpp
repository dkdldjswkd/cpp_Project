#ifndef UNICODE
#define UNICODE
#endif

#include <winsock2.h>
#include <stdio.h>
#include <windows.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int wmain(void)
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %ld\n", iResult);
        return 1;
    }


    SOCKET ListenSocket;
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 논 블락 IO 전환
    u_long io_mode = 1;
    if (0 != ioctlsocket(ListenSocket, FIONBIO, &io_mode))
        throw;

    sockaddr_in service;
    service.sin_family = AF_INET;
#pragma warning(suppress : 4996)
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(27015);

    if (bind(ListenSocket,
        (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(ListenSocket, 1) == SOCKET_ERROR) {
        wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    SOCKET AcceptSocket;
    wprintf(L"Waiting for client to connect...\n");

    AcceptSocket = accept(ListenSocket, NULL, NULL);
    if (AcceptSocket == INVALID_SOCKET) {
        wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
    }
    else
        wprintf(L"Client connected.\n");

    printf("호출 되나");

    closesocket(ListenSocket);

    WSACleanup();
    return 0;
}