#include <WS2tcpip.h>
#include <WinSock2.h>
#include "LanServer.h"
#include "protocol.h"
#include "MemoryLogger.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "RingBuffer.lib")
#pragma comment(lib, "ProtocolBuffer.lib")

using namespace std;
using namespace J_LIB;

// �����
extern int sendQ_remain;

//------------------------------
// Server Func
//------------------------------

LanServer::LanServer() : tls_index(TlsAlloc()) {  }
LanServer::~LanServer() {}

bool LanServer::Create_IOCP() {
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, worker_num);
	return (h_iocp != INVALID_HANDLE_VALUE);
}

bool LanServer::Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey) {
	return h_iocp == CreateIoCompletionPort((HANDLE)h_file, h_iocp, completionKey, 0);
}

void LanServer::Init(WORD worker_num, WORD port, DWORD max_session){
	this->worker_num = worker_num;
	this->server_port = port;
	this->max_session = max_session;

	// set session array
	session_array = new Session[max_session];

	// set index stack
	for (int i = 0; i < max_session; i++)
		sessionIndex_stack.Push(max_session - 1 - i);
}

void LanServer::StartUp(DWORD IP, WORD port, WORD worker_num, bool nagle, DWORD max_session) {
	Init(worker_num, port, max_session);

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		throw;

	listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) throw;

	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = IP;

	// Set nagle
	if (nagle == false) {
		int opt_val = TRUE;
		setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));
	}

	// Set Linger
	LINGER linger = { 1, 0 };
	setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof linger);

	// bind & listen
	if (0 != bind(listen_sock, (SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN))) throw;
	if (0 != listen(listen_sock, SOMAXCONN_HINT(65535))) throw;
	if (!Create_IOCP()) throw;

	// Create Thread
	acceptThread = thread([this] {AcceptFunc(); });
	workerThread_Pool = new thread[worker_num];
	for (int i = 0; i < worker_num; i++) {
		workerThread_Pool[i] = thread([this] {WorkerFunc(); });
	}

	printf("Server Start \n");
}

void LanServer::AcceptFunc() {
	printf("Start Accept Thread \n");

	for (;;) {
		sockaddr_in client_addr;
		int addr_len = sizeof(client_addr);

		//------------------------------
		// Accept
		//------------------------------
		auto accept_sock = accept(listen_sock, (sockaddr*)&client_addr, &addr_len);
		if (accept_sock == INVALID_SOCKET) {
			SocketError_Handling(nullptr, IO_TYPE::ACCEPT);
			continue;
		}

		//------------------------------
		// ���� ���� �Ǵ�
		//------------------------------
		in_addr accept_ip = client_addr.sin_addr;
		WORD accept_port = ntohs(client_addr.sin_port);
		if (false == OnConnectionRequest(accept_ip, accept_port)) {
			closesocket(accept_sock);
			continue;
		}

		//------------------------------
		// ���� �Ҵ�
		//------------------------------

		// ID �Ҵ� (�ִ� ���� �ʰ� �� ���� ����)
		SESSION_ID session_id = Get_SessionID();
		if (session_id == INVALID_SESSION_ID) {
			closesocket(accept_sock);
			continue;
		}

		// ���� �ڷᱸ�� �Ҵ�
		Session* p_accept_session = &session_array[session_id.session_index];
		p_accept_session->Set(accept_sock, accept_ip, accept_port, session_id);
		auto ret = LanServer::Bind_IOCP(accept_sock, (ULONG_PTR)p_accept_session);
		accept_tps++;

		//------------------------------
		// ���� �α��� �� �۾�
		//------------------------------
		OnClientJoin(session_id.session_id);
		InterlockedIncrement((LONG*)&session_count);

		//------------------------------
		// WSARecv
		//------------------------------
		AsyncRecv(p_accept_session);

		// ���� I/O Count ����
		DecrementIOCount(p_accept_session);
	}

	printf("End Accept Thread \n");
}

void LanServer::WorkerFunc() {
	static LONG thread_id = 0x90;
	InterlockedAdd(&thread_id, 0x10);
	TlsSetValue(tls_index, (LPVOID)thread_id);

	printf("Start Worker Thread \n");

	for (;;) {
		DWORD	io_size;// = 0;
		Session* p_session;// = 0;
		LPOVERLAPPED p_overlapped;// = 0;

		BOOL ret_GQCS = GetQueuedCompletionStatus(h_iocp, &io_size, (PULONG_PTR)&p_session, &p_overlapped, INFINITE);

		// ��Ŀ ������ ����
		if (io_size == 0 && p_session == 0 && p_overlapped == 0) {
			PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
			return;
		}
		// FIN
		if (io_size == 0) {
			IF_CRASH(&p_session->send_overlapped == p_overlapped);
			goto Decrement_IOCount;
		}
		// IOCP ��ť�� ���� (IOCP ����)
		if (p_overlapped == 0) CRASH();

		// recv �Ϸ�����
		if (&p_session->recv_overlapped == p_overlapped) {
			if (FALSE == ret_GQCS)  CRASH();  //SocketError_Handling(p_session, IO_TYPE::RECV);
			else {
				p_session->recv_buf.Move_Rear(io_size);
				Recv_Completion(p_session);
			}
		}
		// send �Ϸ�����
		else if (&p_session->send_overlapped == p_overlapped) {
			if (FALSE == ret_GQCS) CRASH(); //SocketError_Handling(p_session, IO_TYPE::SEND);
			else {
				Send_Completion(p_session);
			}
		}
		else {
			CRASH();
		}

	Decrement_IOCount:
		DecrementIOCount(p_session);
	}

	printf("End Worker Thread \n");
}

bool LanServer::Release_Session(Session* p_session){
	if (0 < p_session->io_count)
		return false;

	if (0 == InterlockedCompareExchange64((long long*)&p_session->release_flag, 1, 0)) {
		PacketBuffer* packet;
		while (p_session->sendQ.Dequeue(&packet)) {
			InterlockedDecrement((LONG*)&sendQ_remain);
			PacketBuffer::Free(packet);
		}
		for (int i = 0; i < p_session->sendPacket_count; i++) {
			PacketBuffer::Free(p_session->sendPacket_array[i]);
		}

		closesocket(p_session->sock);
		sessionIndex_stack.Push(p_session->session_id.session_index);
		InterlockedDecrement((LONG*)&session_count);
		return true;
	}

	return false;
}

void LanServer::Send_Completion(Session* p_session){
	// Send �Ϸ�� Packet ��ȯ �� send flag off
	for (int i = 0; i < p_session->sendPacket_count; i++) {
		PacketBuffer::Free(p_session->sendPacket_array[i]);
		InterlockedIncrement(&sendMsg_tps);
	}
	p_session->sendPacket_count = 0;

	if (InterlockedExchange8((char*)&p_session->send_flag, false) == false)
		CRASH();

	if (p_session->disconnect_flag)
		return;
	if (p_session->sendQ.GetUseCount() <= 0)
		return;
	
	SendPost(p_session);
}

// SendQ Enqueue (��Ʈ��ũ ��� �߰� �� SendQ Enqueue, ��쿡 ���� WSASend() call)
bool LanServer::SendPacket(SESSION_ID session_id, PacketBuffer* send_packet) {
	Session* p_session = Check_InvalidSession(session_id);
	if (nullptr == p_session) 
		return false;

	if (p_session->disconnect_flag) {
		DecrementIOCount(p_session);
		return false;
	}

	// ��Ŷ ����� ����
	send_packet->Set_LanHeader();
	send_packet->Increment_refCount();
	p_session->sendQ.Enqueue(send_packet);
	InterlockedIncrement((LONG*)&sendQ_remain);

	bool send_success = SendPost(p_session);
	DecrementIOCount(p_session);
	return send_success;
}

// Send �õ�'��' �� (Send ���� 1. Send ������ X, 2. SendQ not empty)
bool LanServer::SendPost(Session* p_session) {
	// Empty return
	if (p_session->sendQ.GetUseCount() <= 0)
		return false;

	// Send 1ȸ üũ (send flag, true �� send ���� ��)
	if (p_session->send_flag == true)
		return false;
	if (InterlockedExchange8((char*)&p_session->send_flag, true) == true)
		return false;

	// Empty continue
	if (p_session->sendQ.GetUseCount() <= 0) {
		InterlockedExchange8((char*)&p_session->send_flag, false);
		return SendPost(p_session);
	}

	AsyncSend(p_session);
	return true;
}

// WSASend ���� (* ���� ���� �׵� �� Call �Ұ�. ��ȿ�� �����̶� ����, ���� ���� X)
int LanServer::AsyncSend(Session* p_session) {
	WSABUF wsaBuf[MAX_SEND_MSG];

	for (int i = 0; i < MAX_SEND_MSG; i++) {
		if (p_session->sendQ.GetUseCount() <= 0) {
			p_session->sendPacket_count = i;
			break;
		}

		InterlockedDecrement((LONG*)&sendQ_remain);
		p_session->sendQ.Dequeue((PacketBuffer**)&p_session->sendPacket_array[i]);
		wsaBuf[i].buf = p_session->sendPacket_array[i]->Get_Packet();
		wsaBuf[i].len = p_session->sendPacket_array[i]->Get_PacketSize();
	}
	if (p_session->sendPacket_count == 0) {
		p_session->sendPacket_count = MAX_SEND_MSG;
		DisconnectSession(p_session);
		return false;
	}

	ZeroMemory(&p_session->send_overlapped, sizeof(p_session->send_overlapped));
	IncrementIOCount(p_session);
	if (SOCKET_ERROR == WSASend(p_session->sock, wsaBuf, p_session->sendPacket_count, NULL, 0, &p_session->send_overlapped, NULL)) {
		auto err_no = WSAGetLastError();

		// Send ����
		if (err_no != ERROR_IO_PENDING) {
			//CRASH();
			DecrementIOCount(p_session);
			DisconnectSession(p_session);
			return false;
		}
	}

	return true;
}

// Recv RingBuffer�� Recv
bool LanServer::AsyncRecv(Session* p_session) {
	WSABUF wsaBuf[2];
	DWORD flags = 0;

	// Recv Remain Pos
	wsaBuf[1].buf = p_session->recv_buf.Get_BeginPos();
	wsaBuf[1].len = p_session->recv_buf.Remain_EnqueueSize();

	// Recv Write Pos
	wsaBuf[0].buf = p_session->recv_buf.Get_WritePos();
	wsaBuf[0].len = p_session->recv_buf.Direct_EnqueueSize();

	ZeroMemory(&p_session->recv_overlapped, sizeof(p_session->recv_overlapped));
	IncrementIOCount(p_session);
	if (SOCKET_ERROR == WSARecv(p_session->sock, wsaBuf, 2, NULL, &flags, &p_session->recv_overlapped, NULL)) {
		// Recv ����
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			DecrementIOCount(p_session);
			return false;
		}
	}
	// Recv ����
	if (p_session->disconnect_flag) {
		// ������ �� ������, ������� (dicconnect flag�� �����־��� ������)
		CancelIoEx((HANDLE)p_session->sock, NULL);
	}

	return true;
}

int LanServer::SocketError_Handling(Session* p_session, IO_TYPE io_type){
	//printf("unique (%d) I/O(%d) Error no : %d / (send 1, recv 2, accept 3)", p_session->session_id.session_unique, io_type, WSAGetLastError());
	// ip, id, io type, error num �α�
	return 0;
}

void LanServer::Recv_Completion(Session* p_session){
	// ��Ŷ ����
	for (;;) {
		int recv_len = p_session->recv_buf.Get_UseSize();
		if (recv_len <= LAN_HEADER_SIZE)
			break;

		LAN_HEADER lanHeader;
		p_session->recv_buf.Peek(&lanHeader, LAN_HEADER_SIZE);

		// ���̷ε� ������ ����
		if (recv_len < lanHeader.len + LAN_HEADER_SIZE)
			break;

		//------------------------------
		// OnRecv (��Ʈ��ũ ��� ����)
		//------------------------------
		PacketBuffer* contents_packet = PacketBuffer::Alloc_LanPacket();

		// ������ ��Ŷ ����
		p_session->recv_buf.Move_Front(LAN_HEADER_SIZE);
		p_session->recv_buf.Dequeue(contents_packet->Get_writePos(), lanHeader.len);
		contents_packet->Move_Wp(lanHeader.len);

		// ����� ��Ŷ ó��
		OnRecv(p_session->session_id, contents_packet);
		InterlockedIncrement(&recvMsg_tps);

		auto ret = PacketBuffer::Free(contents_packet);
		IF_CRASH(ret != 0);
	}

	//------------------------------
	// Post Recv (Recv �ɾ�α�)
	//------------------------------
	if (false == p_session->disconnect_flag) {
		AsyncRecv(p_session);
	}
}

Session* LanServer::Check_InvalidSession(SESSION_ID session_id) {
	Session* p_session = &session_array[session_id.session_index];

	InterlockedIncrement((LONG*)&p_session->io_count);
	// ���� ������ ����
	if (true == p_session->release_flag) {
		DecrementIOCount(p_session);
		return nullptr;
	}
	// ���� ����
	if (p_session->session_id != session_id) {
		DecrementIOCount(p_session);
		return nullptr;
	}

	// ���� ���ɼ� ����
	return p_session;
}

bool LanServer::Disconnect(SESSION_ID session_id) {
	Session* p_session = Check_InvalidSession(session_id);
	if (nullptr == p_session) return true;
	DisconnectSession(p_session);
	DecrementIOCount(p_session);
	return true;
}

void LanServer::CleanUp(void) {
	// AcceptThread ����
	if (acceptThread.joinable()) {
		acceptThread.join();
	}

	// WorkerThread ����
	//PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
	for (int i = 0; i < worker_num; i++) {
		if (workerThread_Pool[i].joinable()) {
			workerThread_Pool[i].join();
		}
	}

	// ���� ����
	// ...

	closesocket(listen_sock);
	CloseHandle(h_iocp);
	WSACleanup();
}

void LanServer::PrintTPS() {
	printf("\
worker num		: %d \n\
session_count	: %d \n\
accept_tps		: %d \n\
recvMsg_tps		: %d \n\
sendMsg_tps		: %d \n\
Packet Count	: %d \n\
sendQ remain	: %d \n\
\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n",
worker_num, session_count, accept_tps, recvMsg_tps, sendMsg_tps, PacketBuffer::GetUseCount(), sendQ_remain);

	accept_tps = 0;
	recvMsg_tps = 0;
	sendMsg_tps = 0;
}

//------------------------------
// SESSION_ID
//------------------------------

SESSION_ID LanServer::Get_SessionID() {
	static DWORD unique = 1;

	DWORD index;
	if (false == sessionIndex_stack.Pop(&index)) {
		return INVALID_SESSION_ID;
	}

	SESSION_ID session_id(index, unique++);
	return session_id;
}

//------------------------------
// SESSION ID
//------------------------------

SESSION_ID::SESSION_ID(){}
SESSION_ID::SESSION_ID(UINT64 value) { session_id = value; }
SESSION_ID::SESSION_ID(DWORD index, DWORD unique_no) { session_index = index, session_unique = unique_no; }
SESSION_ID::~SESSION_ID(){}

void SESSION_ID::operator=(const SESSION_ID& other) {
	session_id = other.session_id;
}

void SESSION_ID::operator=(UINT64 value) {
	session_id = value;
}

bool SESSION_ID::operator==(UINT64 value) {
	return session_id == value;
}

SESSION_ID::operator UINT64() {
	return session_id;
}

//------------------------------
// Session
//------------------------------

Session::Session() {
	sock = INVALID_SOCKET;
	ip.s_addr = 0;
	port = 0;
	session_id = INVALID_SESSION_ID;
	send_flag = false;
	disconnect_flag = false;
	sendPacket_count = 0;

	io_count = 0;
	release_flag = false;
}
Session::~Session() {}

void Session::Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id){
	this->sock = sock;
	this->ip = ip;
	this->port = port;
	this->session_id = session_id;
	recv_buf.Clear();
	send_flag = false;
	disconnect_flag = false;
	sendPacket_count = 0;

	// �������� ���� ������ �Ǵ°��� ����
	InterlockedIncrement(&io_count);
	this->release_flag = false;
}