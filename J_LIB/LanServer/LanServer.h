#pragma once

#include <Windows.h>
#include <stack>
#include <thread>
#include <mutex>
#include "PacketBuffer.h"
#include "RingBuffer.h"
#include "LFQueue.h"

#define				MAX_SEND_MSG		100
constexpr UINT64	INVALID_SESSION_ID = 0;

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
	// ���� ������
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
	// ��Ʈ��ũ
	SOCKET listen_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	WORD server_port = 0;

	// ����
	Session* session_array;
	DWORD max_session;
	std::stack<WORD> sessionIndex_stack;
	std::mutex index_lock;

	// ������
	WORD worker_num;
	std::thread* workerThread_Pool;
	std::thread acceptThread;

	// ����͸�
	alignas(32) DWORD session_count = 0;
	alignas(32) DWORD accept_tps = 0;
	alignas(32) DWORD recvMsg_tps = 0;
	alignas(32) DWORD sendMsg_tps = 0;

private:
	// Set
	void Init(WORD worker_num, WORD port, DWORD max_session);
	bool Create_IOCP();
	bool Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey);

	// ����
	SESSION_ID Get_SessionID();

	// ������
	void WorkerFunc();
	void AcceptFunc();

	// IO �Ϸ� ���� ��
	void Recv_Completion(Session* p_session);
	void Send_Completion(Session* p_session);

	// Send/Recv
	bool Post_Recv(Session* p_session);
	bool Post_Send(Session* p_session);
	int	IOCP_Send(Session* p_session);
	int	IOCP_Recv(Session* p_session);
	int	SocketError_Handling(Session* p_session, IO_TYPE io_type);

	// ������
	void Release_Session(Session* p_session);

public:
	void StartUp(DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session);
	void CleanUp();
	int  Get_SessionCount();
	bool Send_Packet(SESSION_ID session_id, J_LIB::PacketBuffer* send_packet);
	void Monitoring();

// ���̺귯�� ����� �� ������ �Ͽ� ���
protected:
	virtual bool OnConnectionRequest(in_addr IP, WORD Port) = 0;	//  accept ���� ȣ��, client ���� ���� ��ȯ
	virtual void OnClientJoin(SESSION_ID session_id) = 0;		// ���� ó�� �� ȣ��
	virtual void OnRecv(SESSION_ID session_id, J_LIB::PacketBuffer* contents_packet) = 0;
	virtual void OnClientLeave(SESSION_ID session_id) = 0; // Release �� ȣ��
	virtual void OnError(int errorcode /* (wchar*) */) = 0;

	//virtual void OnSend(SessionID, int sendsize) = 0;           // ��Ŷ �۽� �Ϸ� ��
	//virtual void OnWorkerThreadBegin() = 0;                    // ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	//virtual void OnWorkerThreadEnd() = 0;                      // ��Ŀ������ 1���� ���� ��
};

enum class IO_TYPE : BYTE {
	NONE,
	SEND,
	RECV,
	ACCEPT,
};