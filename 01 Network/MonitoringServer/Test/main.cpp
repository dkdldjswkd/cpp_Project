#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <WS2tcpip.h>
#include <memory.h>
#include <timeapi.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main() {
	sockaddr_in server_address;
	if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1) {
	}
}