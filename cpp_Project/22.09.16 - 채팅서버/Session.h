#pragma once
#include "NetworkDefine.h"
#include "Session_NickName.h"

#include "../../J_LIB/RingBuffer/RingBuffer.h"
#pragma comment(lib,  "../../J_LIB/RingBuffer/RingBuffer.lib")

struct Session{
	Session();
	~Session();

public:
	SOCKET sock = INVALID_SOCKET;
	ULONG ip = 0;
	USHORT port = 0;
	unsigned short id = INVALID_ID;

	RingBuffer send_buf;
	RingBuffer recv_buf;

	bool disconnect_flag = false;

public:
	void Set(SOCKET s, ULONG ip, USHORT port, unsigned short id);
	void Disconnect();
	void Clear();


	// ÄÁÅÙÃ÷
public:
	// ´Ğ³×ÀÓ
	NickName nickName;
	int no = 0;
};

inline unsigned short Get_Session_ID() {
	static unsigned short id = 1;
	return id++;
}

struct ID_Compare {
	bool operator() (const Session* const a, const Session* const b) const {
		return a->id < b->id;
	}
};