#include <iostream>
#include <winsock2.h> 
#include <WS2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

BOOL DomainToIP(WCHAR* szDomain, IN_ADDR* pAddr, int& no_domain) {
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* pSockAddr;

	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0)
		return FALSE;

	no_domain = 0;
	for (; pAddrInfo; pAddrInfo = pAddrInfo->ai_next) {
		pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;

		*pAddr = pSockAddr->sin_addr;
		pAddr++;
		no_domain++;
	}

	FreeAddrInfo(pAddrInfo);
	return TRUE;
}

int main() {
	WSADATA wsaData;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		printf("if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) \n");
		printf("ERROR : %d", WSAGetLastError());
	}

	WCHAR domain_name[20] = L"naver.com";
	IN_ADDR in_addr[10];
	int no_domain;
	DomainToIP(domain_name, in_addr, no_domain);

	for (int i = 0; i < no_domain; i++) {
		printf("%d.", in_addr[i].S_un.S_un_b.s_b1);
		printf("%d.", in_addr[i].S_un.S_un_b.s_b2);
		printf("%d.", in_addr[i].S_un.S_un_b.s_b3);
		printf("%d", in_addr[i].S_un.S_un_b.s_b4);

		printf("\n\n");
	}

	WSACleanup();
}