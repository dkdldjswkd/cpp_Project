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
	static enum class NetType : BYTE {
		LAN,
		NET,
		NONE,
	};

	static enum class PQCS_TYPE : BYTE {
		SEND_POST = 1,
		RELEASE_SESSION = 2,
		NONE = 3,
	};

private:
	// ��������
	BYTE protocolCode;
	BYTE privateKey;

	// ��Ʈ��ũ
	NetType netType;
	SOCKET listenSock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	WORD serverPort = 0;

	// ����
	Session* sessionArray;
	DWORD maxSession;
	LFStack<DWORD> sessionIdxStack;

	// ������
	WORD maxWorker;
	WORD activeWorker;
	std::thread* workerThreadPool;
	std::thread acceptThread;
	std::thread timeoutThread;

	// �ɼ�
	bool nagleFlag;
	bool timeoutFlag;

	// Ÿ�Ӿƿ�
	DWORD timeoutCycle;
	DWORD timeout;

	// ����͸�
	DWORD acceptTPS = 0;
	DWORD acceptTotal = 0;
	alignas(64) DWORD sessionCount = 0;
	alignas(64) DWORD recvMsgTPS = 0;
	alignas(64) DWORD sendMsgTPS = 0;

public:
	// ��ƿ
	Parser parser;

private:
	// ������
	void WorkerFunc();
	void AcceptFunc();
	void TimeOutFunc();

	// IO �Ϸ� ���� ��ƾ
	void RecvCompletion_LAN(Session* p_session);
	void RecvCompletion_NET(Session* p_session);
	void SendCompletion(Session* p_session);

	// ����
	SESSION_ID GetSessionID();
	Session* Check_InvalidSession(SESSION_ID session_id); // * public �Լ� ���ο����� ���
	inline void DecrementIOCount(Session* p_session);
	inline void IncrementIOCount(Session* p_session);
	inline void DisconnectSession(Session* p_session);

	// Send/Recv
	bool SendPost(Session* p_session);
	int	 AsyncSend(Session* p_session);
	bool AsyncRecv(Session* p_session);

	// ������
	bool ReleaseSession(Session* p_session);

protected:
	// ���̺귯�� ����� �� ������ �Ͽ� ���
	virtual bool OnConnectionRequest(in_addr IP, WORD Port) = 0;
	virtual void OnClientJoin(SESSION_ID session_id) = 0;		
	virtual void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet) = 0;
	virtual void OnClientLeave(SESSION_ID session_id) = 0;
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;          
	// virtual void OnWorkerThreadBegin() = 0;                    
	// virtual void OnWorkerThreadEnd() = 0;                      

public:
	void StartUp();
	void CleanUp();

public:
	void SendPacket(SESSION_ID session_id, PacketBuffer* send_packet);
	bool Disconnect(SESSION_ID session_id);

public:
	// ����͸� Getter
	DWORD GetSessionCount();
	DWORD GetAcceptTotal();
	DWORD GetAcceptTPS();
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

// * 'IO Count == 0' �� �� �� ������ ����Ұ�. (�׷��� �ʴٸ�, �ٸ����� ���¹��� �߻�)
inline void NetServer::DisconnectSession(Session* p_session) {
	p_session->disconnect_flag = true;
	CancelIoEx((HANDLE)p_session->sock, NULL);
}
inline DWORD NetServer::GetSessionCount() {
	return sessionCount;
}
inline DWORD NetServer::GetAcceptTotal() {
	return acceptTotal;
}
inline DWORD NetServer::GetAcceptTPS() {
	auto tmp = acceptTPS;
	acceptTPS = 0;
	return tmp;
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