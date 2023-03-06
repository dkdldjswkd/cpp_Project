#include "stdafx.h"
#include "Session.h"

Session::Session() {}

Session::~Session() {
}

void Session::Set(SOCKET s, ULONG ip, USHORT port, unsigned short id) {
	sock = s;
	this->ip = ip;
	this->port = port;
	this->id = id;
	this->disconnect_flag = false;

	this->send_buf.Clear();
	this->recv_buf.Clear();
}

void Session::Disconnect() {
	printf("[SOCKET : %d, id : %d] CLOSE \n", (int)this->sock, this->id);

	closesocket(this->sock);
	this->sock = INVALID_SOCKET;
}

void Session::Clear(){

}
