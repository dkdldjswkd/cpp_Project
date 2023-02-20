#include <cpp_redis/cpp_redis>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>  // For _CrtSetReportMode
#include <errno.h>
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
	char chattingServerIP[5] = { '1' , '1' ,'1' , '1' ,'1' };
	char src[100] = "12";
	strncpy_s(chattingServerIP, sizeof(chattingServerIP), src, 2);
	cout << chattingServerIP << endl;
}