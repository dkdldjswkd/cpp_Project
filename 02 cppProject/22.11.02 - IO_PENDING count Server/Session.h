#pragma once
#include <WinSock2.h>
#include "Define.h"
#include "../../J_LIB/RingBuffer/RingBuffer.h"

struct Session {
public:
	Session();

public:
	WSAOVERLAPPED overlapped;
	SOCKET sock = INVALID_SOCKET;
	WSABUF wsa_recv;
	WSABUF wsa_send;
	char recv_buf[1024];
	char send_buf[1024];
	SESSION_ID id = INVALID_SESSION_ID;

public:
	void Clear();
	void Set(SOCKET sock, SESSION_ID id);

// static Func
public:
	static unsigned int Get_SessionID();
};

