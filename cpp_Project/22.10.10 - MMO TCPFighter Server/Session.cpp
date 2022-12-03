#include "stdafx.h"
#include "Session.h"
using namespace std;

// ------------------------------
// Session
// ------------------------------

Session::Session() {
}

Session::~Session() {
}

void Session::Clear(DWORD accept_frame){
	recv_buf.Clear();
	send_buf.Clear();
	flag_disconnect = false;

	recv_accept = false;
	this->accept_frame = accept_frame;
}

void Session::Set(SOCKET sock, SESSION_ID session_id, DWORD last_recvTime){
	this->sock = sock;
	this->session_id = session_id;
	this->last_recvTime = last_recvTime;
}

SESSION_ID Get_SessionID(){
	static SESSION_ID id = 1;
	if (id == INVALID_SESSION_ID) id++;
	return id++;
}