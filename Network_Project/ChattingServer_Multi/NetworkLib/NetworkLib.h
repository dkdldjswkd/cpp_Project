#pragma once
// ���� 6:46 2023-02-11
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

	// ���� ���۷��� ī��Ʈ ���� (release_flag, io_count�� ���� ĳ�ö��ο� ��ġ�ϰ� �ǵ�)
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
	// ��Ʈ��ũ
	NetworkArea networkArea;
	SOCKET listen_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	WORD server_port = 0;

	// ����
	Session* session_array;
	DWORD max_session;
	LFStack<DWORD> sessionIndex_stack;

	// ������
	WORD maxWorkerNum;
	WORD concurrentWorkerNum;

	std::thread* workerThread_Pool;
	std::thread acceptThread;

private:
	// ����͸�
	alignas(64) DWORD sessionCount = 0;
	alignas(64) DWORD recvMsgTPS = 0;
	alignas(64) DWORD sendMsgTPS = 0;
	DWORD acceptTPS = 0;
	DWORD acceptTotal = 0;

private:
	// Set
	void Init(WORD maxWorker, WORD releaseWorker, WORD port, DWORD max_session);
	bool Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey);

	// ������
	void WorkerFunc();
	void AcceptFunc();

	// IO �Ϸ� ���� ��ƾ
	void (NetworkLib::* RecvCompletion)(Session* p_session);
	void RecvCompletion_LAN(Session* p_session);
	void RecvCompletion_NET(Session* p_session);
	void SendCompletion(Session* p_session);

	// ����
	SESSION_ID Get_SessionID();
	Session* Check_InvalidSession(SESSION_ID session_id);
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
	virtual bool OnConnectionRequest(in_addr IP, WORD Port) = 0;	//  accept ���� ȣ��, client ���� ���� ��ȯ
	virtual void OnClientJoin(SESSION_ID session_id) = 0;		// ���� ó�� �� ȣ��
	virtual void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet) = 0;
	virtual void OnClientLeave(SESSION_ID session_id) = 0; // Release �� ȣ��
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;           // ��Ŷ �۽� �Ϸ� ��
	// virtual void OnWorkerThreadBegin() = 0;                    // ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	// virtual void OnWorkerThreadEnd() = 0;                      // ��Ŀ������ 1���� ���� ��

public:
	void StartUp(NetworkArea area, DWORD IP, WORD port, WORD maxWorker, WORD releaseWorker, bool nagle, DWORD maxSession);
	void CleanUp();

public:
	void SendPacket(SESSION_ID session_id, PacketBuffer* send_packet);
	bool Disconnect(SESSION_ID session_id);

public:
	// ����͸� Getter
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

// * 'IO Count == 0' �� �� �� ������ ����Ұ�. (�׷��� �ʴٸ�, �ٸ����� ���¹��� �߻�)
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