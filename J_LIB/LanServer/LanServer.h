#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include "PacketBuffer.h"
#include "RingBuffer.h"
#include "LFQueue.h"
#include "LFStack.h"

#define				MAX_SESSION			20000
#define				MAX_SEND_MSG		100
constexpr UINT64	INVALID_SESSION_ID = -1;

class ProtocolBuffer;
enum class IO_TYPE : BYTE;

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
	//------------------------------
	// 대입 연산자
	//------------------------------
	void operator=(const SESSION_ID& other);
	void operator=(UINT64 value);
	bool operator==(UINT64 value);
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
	SOCKET sock = INVALID_SOCKET;
	in_addr ip;
	WORD port;

	SESSION_ID	session_id = INVALID_SESSION_ID;
	bool send_flag = false;

	// 세션 레퍼런스 카운트 역할
	alignas(8) BOOL release_flag = false;
	LONG io_count = 0;

	J_LIB::PacketBuffer* sendPacket_array[MAX_SEND_MSG];
	LONG sendPacket_count = 0;

	OVERLAPPED recv_overlapped = { 0, };
	OVERLAPPED send_overlapped = { 0, };
	RingBuffer recv_buf;
	LFQueue<J_LIB::PacketBuffer*> sendQ;

public:
	void Clear();
	void Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id);
};
typedef Session* PSession;

//------------------------------
// LanServer
//------------------------------
class LanServer {
public:
	LanServer();
	virtual ~LanServer();

private:
	// 네트워크
	SOCKET listen_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	WORD server_port = 0;

	// 세션
	Session *session_array;
	DWORD max_session;
	LFStack<DWORD> sessionIndex_stack;

	// 스레드
	WORD worker_num;
	std::thread* workerThread_Pool;
	std::thread acceptThread;

	// 모니터링
	alignas(64) DWORD session_count = 0;
	alignas(64) DWORD accept_tps = 0;
	alignas(64) DWORD recvMsg_tps = 0;
	alignas(64) DWORD sendMsg_tps = 0;

private:
	// Set
	void Init(WORD worker_num, WORD port, DWORD max_session);
	bool Create_IOCP();
	bool Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey);

	// 세션
	SESSION_ID Get_SessionID();
	Session* Check_InvalidSession(SESSION_ID session_id);

	// 스레드
	void WorkerFunc();
	void AcceptFunc();

	// IO 완료 통지 루틴
	void Recv_Completion(Session* p_session);
	void Send_Completion(Session* p_session);

	// Send/Recv
	bool SendPost(Session* p_session);
	int	AsyncSend(Session* p_session);
	int	AsyncRecv(Session* p_session);
	int	SocketError_Handling(Session* p_session, IO_TYPE io_type);

	// 컨텐츠
	bool Release_Session(Session* p_session);

public:
	bool SendPacket(SESSION_ID session_id, J_LIB::PacketBuffer* send_packet);

public:
	void StartUp(DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session);
	void CleanUp();
	int  Get_SessionCount();
	void PrintTPS();

// 라이브러리 사용자 측 재정의 하여 사용
protected:
	virtual bool OnConnectionRequest(in_addr IP, WORD Port) = 0;	//  accept 직후 호출, client 수용 여부 반환
	virtual void OnClientJoin(SESSION_ID session_id) = 0;		// 접속 처리 후 호출
	virtual void OnRecv(SESSION_ID session_id, J_LIB::PacketBuffer* contents_packet) = 0;
	virtual void OnClientLeave(SESSION_ID session_id) = 0; // Release 후 호출
	virtual void OnError(int errorcode /* (wchar*) */) = 0;

	//virtual void OnSend(SessionID, int sendsize) = 0;           // 패킷 송신 완료 후
	//virtual void OnWorkerThreadBegin() = 0;                    // 워커스레드 GQCS 바로 하단에서 호출
	//virtual void OnWorkerThreadEnd() = 0;                      // 워커스레드 1루프 종료 후
};

enum class IO_TYPE : BYTE {
	NONE,
	SEND,
	RECV,
	ACCEPT,
};