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

enum class NetworkArea : BYTE {
	NONE,
	LAN,
	NET,
};

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
	bool disconnect_flag = false;

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
	void Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id);
};
typedef Session* PSession;

//------------------------------
// NetworkLib
//------------------------------
class NetworkLib {
public:
	NetworkLib();
	virtual ~NetworkLib();

private:
	// 네트워크
	NetworkArea networkArea;
	SOCKET listen_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	WORD server_port = 0;

	// 세션
	Session *session_array;
	DWORD max_session;
	LFStack<DWORD> sessionIndex_stack;

	// 스레드
	WORD worker_num[2];
	std::thread* workerThread_Pool;
	std::thread acceptThread;

	// 모니터링
	alignas(64) DWORD session_count = 0;
	alignas(64) DWORD accept_tps = 0;
	alignas(64) DWORD recvMsg_tps = 0;
	alignas(64) DWORD sendMsg_tps = 0;

private:
	// Set
	void Init(WORD maxWorker, WORD releaseWorker, WORD port, DWORD max_session);
	bool Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey);

	// 스레드
	void WorkerFunc();
	void AcceptFunc();

	// IO 완료 통지 루틴
	void (NetworkLib::*RecvCompletion)(Session* p_session);
	void RecvCompletion_LAN(Session* p_session);
	void RecvCompletion_NET(Session* p_session);
	void SendCompletion(Session* p_session);

	// 세션
	SESSION_ID Get_SessionID();
	Session* Check_InvalidSession(SESSION_ID session_id);
	void DecrementIOCount(Session* p_session);
	void IncrementIOCount(Session* p_session);
	void DisconnectSession(Session* p_session);

	// Send/Recv
	bool SendPost(Session* p_session);
	int	 AsyncSend(Session* p_session);
	bool AsyncRecv(Session* p_session);
	int	 SocketError_Handling(Session* p_session, IO_TYPE io_type);

	// 컨텐츠
	bool Release_Session(Session* p_session);

public:
	bool SendPacket(SESSION_ID session_id, J_LIB::PacketBuffer* send_packet);
	bool Disconnect(SESSION_ID session_id);

public:
	void StartUp(NetworkArea area, DWORD IP, WORD port, WORD maxWorker, WORD releaseWorker, bool nagle, DWORD maxSession);
	void CleanUp();
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

	// 디버깅
public:
	const int tls_index;
};

enum class IO_TYPE : BYTE {
	NONE,
	SEND,
	RECV,
	ACCEPT,
};

inline void NetworkLib::DecrementIOCount(Session* p_session) {
	auto ret = InterlockedDecrement((LONG*)&p_session->io_count);
	IF_CRASH(ret == -1);
	if (ret == 0) {
		Release_Session(p_session);
	}
}

inline void NetworkLib::IncrementIOCount(Session* p_session) {
	InterlockedIncrement((LONG*)&p_session->io_count);
}

// Session 권한 취득 후 사용할것. (Session 재사용 문제 있으면 안됨.)
inline void NetworkLib::DisconnectSession(Session* p_session) {
	p_session->disconnect_flag = true;
	CancelIoEx((HANDLE)p_session->sock, NULL);
}