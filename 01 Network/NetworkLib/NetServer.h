#pragma once
#include <Windows.h>
#include <thread>
#include "Session.h"
#include "PacketBuffer.h"
#include "../../00 lib_jy/RingBuffer.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../../00 lib_jy/LFStack.h"
#include "../../00 lib_jy/Parser.h"

//------------------------------
// NetworkLib
//------------------------------
class NetServer {
public:
	NetServer(const char* systemFile, const char* server);
	virtual ~NetServer();

private:
	friend PacketBuffer;

private:
	enum class NetType : BYTE {
		LAN,
		NET,
		NONE,
	};

	enum class PQCS_TYPE : BYTE {
		SEND_POST = 1,
		RELEASE_SESSION = 2,
		NONE,
	};

private:
	// 프로토콜
	BYTE protocol_code;
	BYTE private_key;

	// 네트워크
	NetType netType;
	SOCKET listen_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	WORD server_port = 0;

	// 세션
	Session* session_array;
	DWORD max_session;
	LFStack<DWORD> sessionIndex_stack;

	// 스레드
	WORD maxWorker;
	WORD activeWorker;
	std::thread* workerThread_Pool;
	std::thread acceptThread;
	std::thread timeOutThread;

	// 옵션
	bool nagle_flag;
	bool timeOut_flag;

	// 타임아웃
	DWORD timeOutCycle;
	DWORD timeOut;

private:
	// 모니터링
	alignas(64) DWORD sessionCount = 0;
	alignas(64) DWORD recvMsgTPS = 0;
	alignas(64) DWORD sendMsgTPS = 0;
	DWORD acceptTPS = 0;
	DWORD acceptTotal = 0;

private:
	// 스레드
	void WorkerFunc();
	void AcceptFunc();
	void TimeOutFunc();

	// IO 완료 통지 루틴
	void (NetServer::* RecvCompletion)(Session* p_session);
	void RecvCompletion_LAN(Session* p_session);
	void RecvCompletion_NET(Session* p_session);
	void SendCompletion(Session* p_session);

	// 세션
	SESSION_ID Get_SessionID();
	Session* ValidateSession(SESSION_ID session_id);
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
	virtual bool OnConnectionRequest(in_addr IP, WORD Port) = 0;
	virtual void OnClientJoin(SESSION_ID session_id) = 0;
	virtual void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet) = 0;
	virtual void OnClientLeave(SESSION_ID session_id) = 0;
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;         
	// virtual void OnWorkerThreadBegin() = 0;                   
	// virtual void OnWorkerThreadEnd() = 0;                     

public:
	// 유틸
	Parser parser;

public:
	void StartUp();
	void CleanUp();

public:
	void SendPacket(SESSION_ID session_id, PacketBuffer* send_packet);
	bool Disconnect(SESSION_ID session_id);

public:
	// 모니터링 Getter
	DWORD GetSessionCount();
	DWORD GetAcceptTPS();
	DWORD GetAcceptTotal();
	DWORD GetSendTPS();
	DWORD GetRecvTPS();
};

inline void NetServer::DecrementIOCount(Session* p_session) {
	if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)p_session, (LPOVERLAPPED)PQCS_TYPE::RELEASE_SESSION);
	}
}

inline void NetServer::IncrementIOCount(Session* p_session) {
	InterlockedIncrement((LONG*)&p_session->io_count);
}

// * 'IO Count == 0' 이 될 수 없을때 사용할것. (그렇지 않다면, 다른세션 끊는문제 발생)
inline void NetServer::DisconnectSession(Session* p_session) {
	p_session->disconnect_flag = true;
	CancelIoEx((HANDLE)p_session->sock, NULL);
}

inline DWORD NetServer::GetSessionCount() {
	return sessionCount;
}
inline DWORD NetServer::GetAcceptTPS() {
	auto tmp = acceptTPS;
	acceptTPS = 0;
	return tmp;
}
inline DWORD NetServer::GetAcceptTotal() {
	return acceptTotal;
}
inline DWORD NetServer::GetSendTPS() {
	auto tmp = sendMsgTPS;
	sendMsgTPS = 0;
	return tmp;
}
inline DWORD NetServer::GetRecvTPS() {
	auto tmp = recvMsgTPS;
	recvMsgTPS = 0;
	return tmp;
}