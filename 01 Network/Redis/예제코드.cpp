#include <iostream>
#include <cpp_redis/cpp_redis>
#pragma comment (lib, "ws2_32.lib")

int main()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);


	cpp_redis::client client;

	client.connect();

	client.set("hello", "42");
	client.get("hello", [](cpp_redis::reply& reply) {
		std::cout << reply << std::endl;
		});
	//! also support std::future
	//! std::future<cpp_redis::reply> get_reply = client.get("hello");

	client.sync_commit();
	//! or client.commit(); for asynchronous call}
}