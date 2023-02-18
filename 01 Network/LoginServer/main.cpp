#include <iostream>
#include "LoginServer.h"
#include "../NetworkLib/CrashDump.h"
using namespace std;

#define IP			INADDR_ANY
#define PORT		30000
#define MAX_SESSION 10000

#define DB_IP		"127.0.0.1"
#define DB_PORT		3306
#define DB_ID		"root"
#define DB_PASSWORD "password"
#define DB_SCHEMA	"accountdb"
#define DB_LOGTIME	200

void StartLoginServer(int maxThread, int releaseThread) {
	LoginServer loginServer(DB_IP, DB_PORT, DB_ID, DB_PASSWORD, DB_SCHEMA);
	loginServer.StartUp(NetType::NET, IP, PORT, maxThread, releaseThread, true, MAX_SESSION, false, 5000, 5000);
	printf("StartLoginServer \n");

	for (;;) {
		// 1초 주기 모니터링
		Sleep(1000);
		printf("NetworkLib -------------------------------------------------- \n");
		printf("sessionCount    : %d \n", loginServer.Get_sessionCount());
		printf("PacketCount     : %d \n", PacketBuffer::Get_UseCount());
		printf("acceptTotal     : %d \n", loginServer.Get_acceptTotal());
		printf("acceptTPS       : %d \n", loginServer.Get_acceptTPS());
		printf("sendMsgTPS      : %d \n", loginServer.Get_sendTPS());
		printf("recvMsgTPS      : %d \n", loginServer.Get_recvTPS());
		printf("LoginServer ------------------------------------------------- \n");
		printf("PlayerCount     : %d \n", loginServer.Get_playerCount());
		printf("PlayerPoolCount : %d \n", loginServer.Get_playerPoolCount());
		printf("\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n");
	}
	loginServer.CleanUp();
}

int main() {
	static CrashDump dump;
	int maxThread;
	int releaseThread;
	printf("max & release Thread num >> ");
	cin >> maxThread >> releaseThread;
	StartLoginServer(maxThread, releaseThread);
}

//#include <cpp_redis/cpp_redis>
//#include <iostream>
//#pragma comment (lib, "ws2_32.lib")
//int main() {
//	WORD version = MAKEWORD(2, 2);
//	WSADATA data;
//	WSAStartup(version, &data);
//
//
//	cpp_redis::client client;
//
//	client.connect();
//
//	//std::string a;
//	struct Token {
//		char a[64];
//	};
//	Token a;
//
//	client.set("hello", "42");
//	client.get("hello", [&a](cpp_redis::reply& reply) {
//		if (reply.is_string()) {
//			#pragma warning(suppress : 4996)
//			strncpy((char*)& a, reply.as_string().c_str(), 64);
//			//printf("a");
//		}
//		});
//	client.sync_commit();
//	printf("%s \n", a);
//	//! also support std::future
//	//! std::future<cpp_redis::reply> get_reply = client.get("hello");
//	//! or client.commit(); for asynchronous call}
//}