#include <cpp_redis/cpp_redis>
#include <iostream>
#include "../../DBConnector/DBConnector.h"
#include "../../NetworkLib/Parser.h"
#include "../../NetworkLib/CrashDump.h"
using namespace std;

struct Token {
	char buf[64];
	char nullChar = 0;
};

//int main()
//{
//
//	client.set("hello", "42");
//	client.get("hello", [](cpp_redis::reply& reply) {
//		std::cout << reply << std::endl;
//		});
//	//! also support std::future
//	//! std::future<cpp_redis::reply> get_reply = client.get("hello");
//
//	client.sync_commit();
//	//! or client.commit(); for asynchronous call}
//}

void main() {
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);

	cpp_redis::client connectorRedis;

	connectorRedis.connect();


	char chattingKey[100];
	snprintf(chattingKey, 100, "%d.chatting", 3);
	Token redisToken;
	connectorRedis.get(chattingKey, [&redisToken](cpp_redis::reply& reply) {
		if (reply.is_string()) {
			#pragma warning(suppress : 4996)
			strncpy((char*)&redisToken, reply.as_string().c_str(), 64);

		}
		});
	connectorRedis.sync_commit();
	connectorRedis.del({ chattingKey });
	connectorRedis.sync_commit();
	cout << (char*)&redisToken << endl;
}