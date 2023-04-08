#pragma once
#include <Windows.h>
#include "../../00 lib_jy/RingBuffer.h"
#include "../../00 lib_jy/LFQueue.h"
#include "PacketBuffer.h"

#define MAX_SEND_MSG		100
#define	INVALID_SESSION_ID	-1

//------------------------------
// SessionId
//------------------------------
union SessionId {
public:
	struct { DWORD index, unique; } s;
	DWORD64	sessionId = 0;
#define		SESSION_INDEX  s.index   
#define		SESSION_UNIQUE s.unique

public:
	SessionId();
	SessionId(DWORD64 value);
	SessionId(DWORD index, DWORD unique_no);
	~SessionId();

public:
	void operator=(const SessionId& other);
	void operator=(DWORD64 value);
	operator DWORD64();
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
	SessionId sessionId = INVALID_SESSION_ID;

	// flag
	bool sendFlag = false;
	bool disconnectFlag = false;

	// Send
	LFQueue<PacketBuffer*> sendQ;
	PacketBuffer* sendPacketArr[MAX_SEND_MSG];
	LONG sendPacketCount = 0;

	// Recv
	RingBuffer recvBuf;

	// TimeOut
	DWORD lastRecvTime;

	// Overlapped
	OVERLAPPED recvOverlapped = { 0, };
	OVERLAPPED sendOverlapped = { 0, };

	// 세션 레퍼런스 카운트 역할 (releaseFlag, ioCount가 연속되는 8byte, 같은 캐시라인에 위치하게 의도)
	alignas(64) BOOL releaseFlag = true;
	LONG ioCount = 0;

public:
	void Set(SOCKET sock, in_addr ip, WORD port, SessionId sessionId);
	// Reset : ReleaseSession()
};
typedef Session* PSession;