#pragma once
#include <Windows.h>
#include "../../00 lib_jy/RingBuffer.h"
#include "../../00 lib_jy/LFQueue.h"
#include "PacketBuffer.h"

#define MAX_SEND_MSG		100
#define	INVALID_SESSION_ID	-1

//------------------------------
// SESSION_ID
//------------------------------
union SESSION_ID {
public:
	struct { DWORD index, unique; } s;
	UINT64	session_id = 0;
#define		session_index  s.index   
#define		session_unique s.unique

public:
	SESSION_ID();
	SESSION_ID(UINT64 value);
	SESSION_ID(DWORD index, DWORD unique_no);
	~SESSION_ID();

public:
	void operator=(const SESSION_ID& other);
	void operator=(UINT64 value);
	operator UINT64();
};

//------------------------------
// Session
//------------------------------
class Session {
public:
	Session();
	~Session();

public:
	// 세션 정보
	SOCKET sock = INVALID_SOCKET;
	in_addr ip;
	WORD port;
	SESSION_ID	session_id = INVALID_SESSION_ID;

	// flag
	bool send_flag = false;
	bool disconnect_flag = false;

	// Send
	LFQueue<PacketBuffer*> sendQ;
	PacketBuffer* sendPacketArr[MAX_SEND_MSG];
	LONG sendPacketCount = 0;

	// Recv
	RingBuffer recv_buf;

	// TimeOut
	DWORD lastRecvTime;

	// Overlapped
	OVERLAPPED recv_overlapped = { 0, };
	OVERLAPPED send_overlapped = { 0, };

	// 세션 레퍼런스 카운트 역할 (release_flag, io_count가 같은 캐시라인에 위치하게 의도)
	alignas(64) BOOL release_flag = true;
	LONG io_count = 0;

public:
	void Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id);
};
typedef Session* PSession;