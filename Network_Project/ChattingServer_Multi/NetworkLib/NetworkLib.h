#pragma once
// 오후 6:46 2023-02-11
#include <Windows.h>
#include <thread>
#include <mutex>
#include "PacketBuffer.h"
#include "RingBuffer.h"
#include "LFQueue.h"
#include "LFStack.h"

#define				MAX_SEND_MSG		100
constexpr UINT64	INVALID_SESSION_ID = -1;

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

	// 세션 레퍼런스 카운트 역할 (release_flag, io_count가 같은 캐시라인에 위치하게 의도)
	alignas(64) BOOL release_flag = false;
	LONG io_count = 0;

	PacketBuffer* sendPacket_array[MAX_SEND_MSG];
	LONG sendPacket_count = 0;

	OVERLAPPED recv_overlapped = { 0, };
	OVERLAPPED send_overlapped = { 0, };
	RingBuffer recv_buf;
	LFQueue<PacketBuffer*> sendQ;

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
	Session* session_array;
	DWORD max_session;
	LFStack<DWORD> sessionIndex_stack;

	// 스레드
	WORD maxWorkerNum;
	WORD concurrentWorkerNum;

	std::thread* workerThread_Pool;
	std::thread acceptThread;

private:
	// 모니터링
	alignas(64) DWORD sessionCount = 0;
	alignas(64) DWORD recvMsgTPS = 0;
	alignas(64) DWORD sendMsgTPS = 0;
	DWORD acceptTPS = 0;
	DWORD acceptTotal = 0;

private:
	// Set
	void Init(WORD maxWorker, WORD releaseWorker, WORD port, DWORD max_session);
	bool Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey);

	// 스레드
	void WorkerFunc();
	void AcceptFunc();

	// IO 완료 통지 루틴
	void (NetworkLib::* RecvCompletion)(Session* p_session);
	void RecvCompletion_LAN(Session* p_session);
	void RecvCompletion_NET(Session* p_session);
	void SendCompletion(Session* p_session);

	// 세션
	SESSION_ID Get_SessionID();
	Session* Check_InvalidSession(SESSION_ID session_id);
	inline void DecrementIOCount(Session* p_session);
	inline void IncrementIOCount(Session* p_session);
	inline void DisconnectSession(Session* p_session);

	// Send/Recv
	bool SendPost(Session* p_session);
	int	 AsyncSend(Session* p_session);
	bool AsyncRecv(Session* p_session);

	// 컨텐츠
	bool ReleaseSession(Session* p_session);

protected:
	// 라이브러리 사용자 측 재정의 하여 사용
	virtual bool OnConnectionRequest(in_addr IP, WORD Port) = 0;	//  accept 직후 호출, client 수용 여부 반환
	virtual void OnClientJoin(SESSION_ID session_id) = 0;		// 접속 처리 후 호출
	virtual void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet) = 0;
	virtual void OnClientLeave(SESSION_ID session_id) = 0; // Release 후 호출
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;           // 패킷 송신 완료 후
	// virtual void OnWorkerThreadBegin() = 0;                    // 워커스레드 GQCS 바로 하단에서 호출
	// virtual void OnWorkerThreadEnd() = 0;                      // 워커스레드 1루프 종료 후

public:
	void StartUp(NetworkArea area, DWORD IP, WORD port, WORD maxWorker, WORD releaseWorker, bool nagle, DWORD maxSession);
	void CleanUp();

public:
	void SendPacket(SESSION_ID session_id, PacketBuffer* send_packet);
	bool Disconnect(SESSION_ID session_id);

public:
	// 모니터링 Getter
	DWORD Get_sessionCount();
	DWORD Get_acceptTPS();
	DWORD Get_acceptTotal();
	DWORD Get_sendTPS();
	DWORD Get_recvTPS();
};

inline void NetworkLib::DecrementIOCount(Session* p_session) {
	if (0 == InterlockedDecrement((LONG*)&p_session->io_count))
		ReleaseSession(p_session);
}

inline void NetworkLib::IncrementIOCount(Session* p_session) {
	InterlockedIncrement((LONG*)&p_session->io_count);
}

// * 'IO Count == 0' 이 될 수 없을때 사용할것. (그렇지 않다면, 다른세션 끊는문제 발생)
inline void NetworkLib::DisconnectSession(Session* p_session) {
	p_session->disconnect_flag = true;
	CancelIoEx((HANDLE)p_session->sock, NULL);
}

inline DWORD NetworkLib::Get_sessionCount() {
	return sessionCount;
}
inline DWORD NetworkLib::Get_acceptTPS() {
	auto tmp = acceptTPS;
	acceptTPS = 0;
	return tmp;
}
inline DWORD NetworkLib::Get_acceptTotal() {
	return acceptTotal;
}
inline DWORD NetworkLib::Get_sendTPS() {
	auto tmp = sendMsgTPS;
	sendMsgTPS = 0;
	return tmp;
}
inline DWORD NetworkLib::Get_recvTPS() {
	auto tmp = recvMsgTPS;
	recvMsgTPS = 0;
	return tmp;
}