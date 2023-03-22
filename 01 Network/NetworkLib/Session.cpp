#include "Session.h"

//------------------------------
// SESSION ID
//------------------------------

SESSION_ID::SESSION_ID() {}
SESSION_ID::SESSION_ID(DWORD64 value) { session_id = value; }
SESSION_ID::SESSION_ID(DWORD index, DWORD unique_no) { session_index = index, session_unique = unique_no; }
SESSION_ID::~SESSION_ID() {}

void SESSION_ID::operator=(const SESSION_ID& other) {
	session_id = other.session_id;
}

void SESSION_ID::operator=(DWORD64 value) {
	session_id = value;
}

SESSION_ID::operator DWORD64() {
	return session_id;
}

//------------------------------
// Session
//------------------------------

Session::Session() {}
Session::~Session() {}

void Session::Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id) {
	this->sock = sock;
	this->ip = ip;
	this->port = port;
	this->session_id = session_id;
	recv_buf.Clear();
	sendFlag = false;
	disconnectFlag = false;
	sendPacketCount = 0;
	lastRecvTime = timeGetTime();

	// 생성하자 마자 릴리즈 되는것을 방지
	InterlockedIncrement(&ioCount);
	this->releaseFlag = false;
}