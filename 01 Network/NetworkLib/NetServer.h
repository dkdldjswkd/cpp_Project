#pragma once
#include <Windows.h>
#include <thread>
#include "Session.h"
#include "PacketBuffer.h"
#include "../../00 lib_jy/RingBuffer.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../../00 lib_jy/LFStack.h"
#include "../../00 lib_jy/Parser.h"

#define PQCS_SEND	1	// 1 : SendPacket::WSASend() worker 스레드 우회, 0 : SendPacket 호출 스레드에서 WSASend() call

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
	BYTE protocolCode;
	BYTE privateKey;

	// 네트워크
	NetType netType;
	SOCKET listenSock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;

	// 세션
	DWORD sessionUnique = 0;
	Session* sessionArray;
	DWORD maxSession;
	LFStack<DWORD> sessionIdxStack;

	// 스레드
	WORD maxWorker;
	WORD activeWorker;
	std::thread* workerThreadArr;
	std::thread acceptThread;
	std::thread timeOutThread;

	// 옵션
	bool timeoutFlag;

	// 타임아웃
	DWORD timeoutCycle;
	DWORD timeOut;

	// 모니터링
	DWORD acceptCount = 0;
	DWORD acceptTPS= 0;
	DWORD acceptTotal = 0;
	DWORD recvMsgTPS = 0;
	DWORD sendMsgTPS = 0;
	alignas(64) DWORD sessionCount = 0;
	alignas(64) DWORD recvMsgCount = 0;
	alignas(64) DWORD sendMsgCount = 0;

public:
	// 유틸
	Parser parser;

private:
	// 스레드
	void WorkerFunc();
	void AcceptFunc();
	void TimeoutFunc();

	// IO 완료 통지 루틴
	void RecvCompletionLan(Session* p_session);
	void RecvCompletionNet(Session* p_session);
	void SendCompletion(Session* p_session);

	// 세션
	SessionId GetSessionId();
	Session* ValidateSession(SessionId sessionId);
	void IncrementIOCount(Session* p_session);
	void DecrementIOCount(Session* p_session);
	void DecrementIOCountPQCS(Session* p_session);
	void DisconnectSession(Session* p_session);
	bool ReleaseSession(Session* p_session);

	// Send/Recv
	bool SendPost(Session* p_session);
	int	 AsyncSend(Session* p_session);
	bool AsyncRecv(Session* p_session);

protected:
	// 라이브러리 사용자 측 재정의 하여 사용
	virtual bool OnConnectionRequest(in_addr ip, WORD port) = 0;
	virtual void OnClientJoin(SessionId sessionId) = 0;
	virtual void OnRecv(SessionId sessionId, PacketBuffer* contentsPacket) = 0;
	virtual void OnClientLeave(SessionId sessionId) = 0;
	virtual void OnServerStop() = 0;
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(sessionId, int sendSize) = 0;         
	// virtual void OnWorkerThreadBegin() = 0;                   
	// virtual void OnWorkerThreadEnd() = 0;                     

public:
	// 서버 ON/OFF
	void Start();
	void Stop();

	// 라이브러리 제공 API
	void SendPacket(SessionId sessionId, PacketBuffer* sendPacket);
	void Disconnect(SessionId sessionId);

	// Getter
	void UpdateTPS();
	DWORD GetSessionCount();
	DWORD GetAcceptTPS();
	DWORD GetAcceptTotal();
	DWORD GetSendTPS();
	DWORD GetRecvTPS();
};

//////////////////////////////
// NetServer Inline Func
//////////////////////////////

inline void NetServer::IncrementIOCount(Session* p_session) {
	InterlockedIncrement((LONG*)&p_session->ioCount);
}

inline void NetServer::DecrementIOCount(Session* p_session) {
	if (0 == InterlockedDecrement((LONG*)&p_session->ioCount)) {
		ReleaseSession(p_session);
	}
}

inline void NetServer::DecrementIOCountPQCS(Session* p_session) {
	if (0 == InterlockedDecrement((LONG*)&p_session->ioCount)) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)p_session, (LPOVERLAPPED)PQCS_TYPE::RELEASE_SESSION);
	}
}

// * 'IO Count == 0' 이 될 수 없을때 사용할것. (그렇지 않다면, 다른세션 끊는문제 발생)
inline void NetServer::DisconnectSession(Session* p_session) {
	p_session->disconnectFlag = true;
	CancelIoEx((HANDLE)p_session->sock, NULL);
}

//////////////////////////////
// Getter
//////////////////////////////

inline void NetServer::UpdateTPS() {
	acceptTPS = acceptCount;
	acceptCount = 0;

	sendMsgTPS = sendMsgCount;
	sendMsgCount = 0;

	recvMsgTPS = recvMsgCount;
	recvMsgCount = 0;
}

inline DWORD NetServer::GetSessionCount() {
	return sessionCount;
}

inline DWORD NetServer::GetAcceptTPS() {
	return acceptTPS;
}

inline DWORD NetServer::GetAcceptTotal() {
	return acceptTotal;
}

inline DWORD NetServer::GetSendTPS() {
	return sendMsgTPS;
}

inline DWORD NetServer::GetRecvTPS() {
	return recvMsgTPS;
}