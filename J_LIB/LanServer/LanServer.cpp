#include <WS2tcpip.h>
#include <WinSock2.h>
#include "LanServer.h"
#include "protocol.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "RingBuffer.lib")
#pragma comment(lib, "ProtocolBuffer.lib")

using namespace std;
using namespace J_LIB;

//------------------------------
// Server Func
//------------------------------

LanServer::LanServer() {}
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
	acceptThread = thread([this] {AcceptFunc();});
	workerThread_Pool = new thread[worker_num];
	for (int i = 0; i < worker_num; i++) {
		workerThread_Pool[i] = thread([this] {WorkerFunc(); });
	}

	printf("Server Start \n");
}

int LanServer::Get_SessionCount(){
	return session_count;
}

void LanServer::WorkerFunc() {
	printf("Start Worker Thread \n");

	for (;;) {
		DWORD	io_size = 0;
		Session* p_session = 0;
		LPOVERLAPPED p_overlapped = 0;

		BOOL ret_GQCS = GetQueuedCompletionStatus(h_iocp, &io_size, (PULONG_PTR)&p_session, &p_overlapped, INFINITE);

		// 워커 스레드 종료
		if (io_size == 0 && p_session == 0 && p_overlapped == 0) {
			PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
			return;
		}
		// FIN
		if (io_size == 0) {
			goto Decrement_IOCount;
		}
		// IOCP 디큐잉 실패 (IOCP 에러)
		if (p_overlapped == 0) CRASH();

		// recv 완료통지
		if (&p_session->recv_overlapped == p_overlapped) {
			if (FALSE == ret_GQCS)  CRASH();  //SocketError_Handling(p_session, IO_TYPE::RECV);
			else {
				p_session->recv_buf.Move_Rear(io_size);
				Recv_Completion(p_session);
			}
		}
		// send 완료통지
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
		if ( InterlockedDecrement( &p_session->io_count ) == 0 ){
			Release_Session(p_session);
			continue;
		}
	}

	printf("End Worker Thread \n");
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
		// 연결 수락 판단
		//------------------------------
		in_addr accept_ip = client_addr.sin_addr;
		WORD accept_port = ntohs(client_addr.sin_port);
		if (false == OnConnectionRequest(accept_ip, accept_port)) {
			closesocket(accept_sock);
			continue;
		}

		//------------------------------
		// 세션 할당
		//------------------------------

		// ID 할당
		SESSION_ID session_id = Get_SessionID();
		if (session_id == INVALID_SESSION_ID) {
			closesocket(accept_sock);
			continue;
		}

		// 세션 자료구조 할당
		Session& accept_session = session_array[session_id.session_index];
		accept_session.Clear();
		accept_session.Set(accept_sock, accept_ip, accept_port, session_id);
		auto ret = LanServer::Bind_IOCP(accept_sock, (ULONG_PTR)&accept_session);
		accept_tps++;
		
		//------------------------------
		// 세션 로그인 시 작업
		//------------------------------
		OnClientJoin(session_id.session_id);
		session_count++;

		//------------------------------
		// WSARecv
		//------------------------------
		AsyncRecv(&accept_session);

		// 생성 I/O Count 차감
		if (InterlockedDecrement((LONG*)&accept_session.io_count) == 0) 
			Release_Session(&accept_session);
	}

	printf("End Accept Thread \n");
}

bool LanServer::Release_Session(Session* p_session){
	if (0 < p_session->io_count)
		return false;

	if (0 == InterlockedCompareExchange64((long long*)&p_session->release_flag, 1, 0)) {
		PacketBuffer* packet;
		while (p_session->sendQ.Dequeue(&packet)) {
			PacketBuffer::Free(packet);
		}
		for (int i = 0; i < p_session->sendPacket_count; i++) {
			PacketBuffer::Free(p_session->sendPacket_array[i]);
		}

		closesocket(p_session->sock);
		sessionIndex_stack.Push(p_session->session_id.session_index);
		session_count--;
		return true;
	}

	return false;
}

void LanServer::Send_Completion(Session* p_session){
	// Send 완료된 Packet 반환 및 send flag off
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

// SendQ Enqueue (네트워크 헤더 추가 후 SendQ Enqueue, 경우에 따라 WSASend() call)
bool LanServer::SendPacket(SESSION_ID session_id, PacketBuffer* send_packet) {
	Session* p_session = Check_InvalidSession(session_id);
	if (nullptr == p_session) 
		return false;

	if (p_session->disconnect_flag) {
		if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
			Release_Session(p_session);
		}
		return false;
	}

	// 패킷 헤더부 생성
	LAN_HEADER lan_header;
	lan_header.len = send_packet->Get_PayloadSize();
	send_packet->Set_LanHeader();
	send_packet->Increment_refCount();
	p_session->sendQ.Enqueue(send_packet);

	bool send_success = SendPost(p_session);
	if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
		Release_Session(p_session);
	}
	return send_success;
}

// Send 시도'만' 함 (Send 조건 1. Send 진행중 X, 2. SendQ not empty)
bool LanServer::SendPost(Session* p_session) {
	// Empty return
	if (p_session->sendQ.GetUseCount() <= 0)
		return false;

	// Send 1회 체크 (send flag, true 시 send 진행 중)
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

bool LanServer::Disconnect(SESSION_ID session_id){
	Session* p_session = Check_InvalidSession(session_id);
	if (nullptr == p_session)
		return true;

	p_session->disconnect_flag = true;
	CancelIoEx((HANDLE)p_session->sock, NULL);

	if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
		Release_Session(p_session);
	}
	return true;
}

// WSASend 래핑 (* 세션 권한 휙득 후 Call 할것. 유효한 세션이라 가정)
int LanServer::AsyncSend(Session* p_session) {
	WSABUF wsaBuf[MAX_SEND_MSG];

	for (int i = 0; i < MAX_SEND_MSG; i++) {
		if (p_session->sendQ.GetUseCount() <= 0) {
			p_session->sendPacket_count = i;
			if (i <= 0) CRASH(); // 0 byte send
			break;
		}

		p_session->sendQ.Dequeue((PacketBuffer**)&p_session->sendPacket_array[i]);
		wsaBuf[i].buf = p_session->sendPacket_array[i]->Get_Packet();
		wsaBuf[i].len = p_session->sendPacket_array[i]->Get_PacketSize();
	}

	InterlockedIncrement((LONG*)&p_session->io_count);
	if (SOCKET_ERROR == WSASend(p_session->sock, wsaBuf, p_session->sendPacket_count, NULL, 0, &p_session->send_overlapped, NULL)) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			if (InterlockedDecrement((LONG*)&p_session->io_count) == 0) {
				Release_Session(p_session); // 사실 상 호출되지 않아야함
			}
			return false;
		}
	}

	return true;
}

// Recv RingBuffer에 Recv
bool LanServer::AsyncRecv(Session* p_session) {
	WSABUF wsaBuf[2];
	DWORD flags = 0;

	// Recv Remain Pos
	wsaBuf[1].buf = p_session->recv_buf.Get_BeginPos();
	wsaBuf[1].len = p_session->recv_buf.Remain_EnqueueSize();

	// Recv Write Pos
	wsaBuf[0].buf = p_session->recv_buf.Get_WritePos();
	wsaBuf[0].len = p_session->recv_buf.Direct_EnqueueSize();

	InterlockedIncrement((LONG*)&p_session->io_count);
	if (SOCKET_ERROR == WSARecv(p_session->sock, wsaBuf, 2, NULL, &flags, &p_session->recv_overlapped, NULL)) {
		// Recv 실패
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			if (InterlockedDecrement((LONG*)&p_session->io_count) == 0) {
				Release_Session(p_session);
			}
			return false;
		}
	}
	// Recv 성공
	if (p_session->disconnect_flag) {
		// 재사용일 수 있으나, 상관없음 (dicconnect flag가 켜져있었기 때문에)
		CancelIoEx((HANDLE)p_session->sock, NULL);
	}

	return true;
}

int LanServer::SocketError_Handling(Session* p_session, IO_TYPE io_type){
	//printf("unique (%d) I/O(%d) Error no : %d / (send 1, recv 2, accept 3)", p_session->session_id.session_unique, io_type, WSAGetLastError());
	// ip, id, io type, error num 로깅
	return 0;
}

void LanServer::Recv_Completion(Session* p_session){
	// 패킷 조립
	for (;;) {
		int recv_len = p_session->recv_buf.Get_UseSize();
		if (recv_len <= LAN_HEADER_SIZE)
			break;

		LAN_HEADER lanHeader;
		p_session->recv_buf.Peek(&lanHeader, LAN_HEADER_SIZE);

		// 페이로드 데이터 부족
		if (recv_len < lanHeader.len + LAN_HEADER_SIZE)
			break;

		//------------------------------
		// OnRecv (네트워크 헤더 제거)
		//------------------------------
		PacketBuffer* contents_packet = PacketBuffer::Alloc_LanPacket();

		// 컨텐츠 패킷 생성
		p_session->recv_buf.Move_Front(LAN_HEADER_SIZE);
		p_session->recv_buf.Dequeue(contents_packet->Get_writePos(), lanHeader.len);
		contents_packet->Move_Wp(lanHeader.len);

		// 사용자 패킷 처리
		OnRecv(p_session->session_id, contents_packet);
		InterlockedIncrement(&recvMsg_tps);

		auto ret = PacketBuffer::Free(contents_packet);
		if (ret == false) CRASH();
	}

	//------------------------------
	// Post Recv (Recv 걸어두기)
	//------------------------------
	if (false == p_session->disconnect_flag) {
		AsyncRecv(p_session);
	}
}

void LanServer::CleanUp(void) {
	// AcceptThread 종료
	if (acceptThread.joinable()) {
		acceptThread.join();
	}

	// WorkerThread 종료
	//PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
	for (int i = 0; i < worker_num; i++) {
		if (workerThread_Pool[i].joinable()) {
			workerThread_Pool[i].join();
		}
	}

	// 세션 정리
	// ...

	closesocket(listen_sock);
	CloseHandle(h_iocp);
	WSACleanup();
}

void LanServer::PrintTPS() {
	printf("\
session_count : %d \n\
accept_tps    : %d \n\
recvMsg_tps   : %d \n\
sendMsg_tps   : %d \n\
Packet Count  : %d \n\
\n\n\n\n\n\n\n\n\n\n \n\n\n\n\n\n\n\n\n\n",
session_count, accept_tps, recvMsg_tps, sendMsg_tps, PacketBuffer::GetUseCount());

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

Session* LanServer::Check_InvalidSession(SESSION_ID session_id){
	Session* p_session = &session_array[session_id.session_index];

	InterlockedIncrement((LONG*)&p_session->io_count);
	// 세션 릴리즈 중
	if (true == p_session->release_flag) {
		if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
			Release_Session(p_session);
			return nullptr;
		}
	}
	// 세션 재사용
	if (p_session->session_id != session_id) {
		if (0 == InterlockedDecrement((LONG*)&p_session->io_count)) {
			Release_Session(p_session);
			return nullptr;
		}
	}

	// 재사용 가능성 제로
	return p_session;
}

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

Session::Session() { Clear(); }
Session::~Session() {}

void Session::Clear() {
	sock = INVALID_SOCKET;
	ip.s_addr = 0;
	port = 0;

	session_id = INVALID_SESSION_ID;
	send_flag = false;
	io_count = 0;
	sendPacket_count = 0;

	ZeroMemory(&recv_overlapped, sizeof(recv_overlapped));
	ZeroMemory(&send_overlapped, sizeof(send_overlapped));

	recv_buf.Clear();
}

void Session::Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id){
	this->sock = sock;
	this->ip = ip;
	this->port = port;
	this->session_id = session_id;
	disconnect_flag = false;

	// 생성하자 마자 릴리즈 되는것을 방지
	InterlockedIncrement(&io_count);
	this->release_flag = false;
}