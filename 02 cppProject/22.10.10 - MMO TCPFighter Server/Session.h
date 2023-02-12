#pragma once
#include <WinSock2.h>
#include "../../J_LIB/RingBuffer/RingBuffer.h"
#pragma comment(lib, "../../J_LIB/RingBuffer/RingBuffer.lib")
#include <unordered_map>

#define INVALID_SESSION_ID 0

typedef DWORD SESSION_ID;

struct Session {
public:
	Session();
	~Session();

public:
	SOCKET sock;
	SESSION_ID session_id;
	RingBuffer recv_buf;
	RingBuffer send_buf;
	DWORD last_recvTime;

	bool flag_disconnect = false;

	bool recv_accept = false;
	int accept_frame = 0;

public:
	void Clear(DWORD accept_frame); // 생성자에 준하는 행동
	void Set(SOCKET sock, SESSION_ID session_id, DWORD last_recvTime); // 데이터 세팅
};

SESSION_ID Get_SessionID();