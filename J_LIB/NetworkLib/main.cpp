#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "EchoServer.h"
#include "ChattingServer_Single.h"
#include "ChattingServer_Multi.h"
#include "CrashDump.h"
#include "Logger.h"
#pragma comment (lib, "Winmm.lib")

#define IP INADDR_ANY

using namespace std;
CrashDump dump;

// EchoServer.cpp, Define 확인
void StartEchoServer(int maxThread, int releaseThread) {
	EchoServer echo_server;
	echo_server.StartUp(NetworkArea::LAN, INADDR_ANY, 6000, maxThread, releaseThread, true, 10000);
	printf("StartEchoServer \n");

	for (;;) {
		Sleep(1000);
	}

	echo_server.CleanUp();
}

//InterlockedIncrement((LONG*)&cattingPacketCount);
extern int cattingPacketCount;
// CattingServer.cpp, Define 확인
void StartChattingServer_Single(int maxThread, int releaseThread) {
	ChattingServer_Single server;
	server.StartUp(NetworkArea::NET, INADDR_ANY, 12001, maxThread, releaseThread, true, 10000);
	printf("StartChattingServer Multi \n");

	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		printf("NetworkLib ---------------------------------------------------- \n");
		printf("sessionCount    : %d \n", server.Get_sessionCount());
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", server.Get_acceptTotal());
		printf("acceptTPS       : %d \n", server.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", server.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", server.Get_recvTPS());
		printf("					                                            \n");
		printf("ChattingServer-Single ----------------------------------------- \n");
		printf("PlayerCount     : %d \n", server.Get_playerCount());
		printf("PlayerPoolCount : %d \n", server.Get_playerPoolCount());
		printf("처리량 -------------------------------------------------------- \n");
		printf("JobPoolCount    : %d \n", server.Get_JobPoolCount());
		printf("JobQueueCount   : %d \n", server.Get_JobQueueCount());
		printf("UpdateTPS       : %d \n", server.Get_updateTPS());
		printf("디버깅 -------------------------------------------------------- \n");
		printf("var             : %d \n", 0);
		printf("\n\n\n\n\n\n\n\n\n\n \n\n");
	}

	server.CleanUp();
}

//// CattingServer.cpp, Define 확인
//void StartChattingServer_Multi(int maxThread, int releaseThread) {
//	ChattingServer_Multi ChattingServer_Multi;
//	ChattingServer_Multi.StartUp(NetworkArea::NET, INADDR_ANY, 12001, maxThread, releaseThread, true, 10000);
//	printf("StartChattingServer Multi \n");
//
//	for (;;) {
//		// 1초 주기 모니터링
//		Sleep(1000);
//		printf("NetworkLib -------------------------------------------------- \n");
//		printf("sessionCount    : %d \n", ChattingServer_Multi.Get_sessionCount());
//		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
//		printf("acceptTotal     : %d \n", ChattingServer_Multi.Get_acceptTotal());
//		printf("acceptTPS       : %d \n", ChattingServer_Multi.Get_acceptTPS());
//		printf("sendMsgTPS      : %d \n", ChattingServer_Multi.Get_sendTPS());
//		printf("recvMsgTPS      : %d \n", ChattingServer_Multi.Get_recvTPS());
//		printf("ChattingServer-Multi ----------------------------------------- \n");
//		printf("PlayerCount     : %d \n", ChattingServer_Multi.Get_playerCount());
//		printf("PlayerPoolCount : %d \n", ChattingServer_Multi.Get_playerPoolCount());
//		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n");
//	}
//
//	ChattingServer_Multi.CleanUp();
//}

#define CHATTING
int main() {
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;

#ifdef CHATTING
	StartChattingServer_Single(maxThread, releaseThread);
#endif
#ifndef CHATTING
	StartEchoServer(maxThread, releaseThread);
#endif
}