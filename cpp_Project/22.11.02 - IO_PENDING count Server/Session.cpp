#include "stdafx.h"
#include "Session.h"
#pragma comment(lib, "../../J_LIB/RingBuffer/RingBuffer.lib")

Session::Session() {
}

void Session::Clear() {
	sock = INVALID_SOCKET;
	id = INVALID_SESSION_ID;
	wsa_send.buf = send_buf;
	wsa_send.len = 1024;
	wsa_recv.buf = recv_buf;
	wsa_recv.len = 1024;
}

void Session::Set(SOCKET sock, SESSION_ID id) {
	this->sock = sock;
	this->id = id;
}

unsigned int Session::Get_SessionID(){
	static unsigned int session_id = 0;
	return ++session_id;
}