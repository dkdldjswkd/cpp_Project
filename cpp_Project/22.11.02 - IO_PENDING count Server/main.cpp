#include "stdafx.h"
#include "Define.h"
#include "Server.h"

using namespace std;

int main() {
	Server::StartUp();

	thread accept_thread(Server::AcceptProc);
	thread recv_thread(Server::RecvProc);

	string str;
	for (;;) {
		cin >> str;
		if (strcmp(str.c_str(), "q") == 0) {
			Server::shutdown = true;
			SetEvent(recv_event);
			break;
		}
	}

	accept_thread.join();
	recv_thread.join();
	Server::CleanUp();
}

/*
APC 에코서버 만들기.
+ IO_PENDING 카운트 하기
*/