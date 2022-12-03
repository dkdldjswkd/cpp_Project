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

void LanServer::Init(WORD worker_num, WORD port, DWORD max_session){
	this->worker_num = worker_num;
	this->server_port = port;
	this->max_session = max_session;

	// set session array
	session_array = new Session[max_session];

	// set index stack
	for (int i = 0; i < max_session; i++)
		sessionIndex_stack.push(max_session - 1 - i);
}

bool LanServer::Bind_IOCP(SOCKET h_file, ULONG_PTR completionKey) {
	return h_iocp == CreateIoCompletionPort((HANDLE)h_file, h_iocp, completionKey, 0);
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
		UINT64	key = 0;
		LPOVERLAPPED p_overlapped = 0;
		Session* p_session = 0;

		BOOL ret_GQCS = GetQueuedCompletionStatus(h_iocp, &io_size, (PULONG_PTR)&key, &p_overlapped, INFINITE);

		// 워커 스레드 종료 (** IO Error X)
		if (io_size == 0 && key == 0 && p_overlapped == 0) {
			PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
			return;
		}

		p_session = (Session*)key;
		
		// FIN
		if (io_size == 0) {
			// 디버깅
			auto ret = WSAGetLastError();
			goto Decrement_IOCount;
		}
		else {
			// recv 완료통지
			if (&p_session->recv_overlapped == p_overlapped) {
				if (FALSE == ret_GQCS) {
					SocketError_Handling(p_session, IO_TYPE::RECV);
					// 디버깅
					//printf("[Recv Complete Error] socket(%llu) \n", p_session->sock);
				}
				else {
					//printf("[ !!완료통지 RECV ] socket(%llu) \n", p_session->sock);
					p_session->recv_buf.Move_Rear(io_size);
					Recv_Completion(p_session);
				}
			}
			// send 완료통지
			else if (&p_session->send_overlapped == p_overlapped) {
				if (FALSE == ret_GQCS) {
					SocketError_Handling(p_session, IO_TYPE::SEND);
					// 디버깅
					//printf("[Send Complete Error] socket(%llu) \n", p_session->sock);
				}
				else {
					//printf("[ !!완료통지 SEND ] socket(%llu) \n", p_session->sock);
					Send_Completion(p_session);
				}
			}
			// 디버깅
			else {
				throw;
			}
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

		in_addr accept_ip = client_addr.sin_addr;
		WORD accept_port = ntohs(client_addr.sin_port);
		if (false == OnConnectionRequest(accept_ip, accept_port)) {
			closesocket(accept_sock);
			continue;
		}

		// 디버깅
		//printf("1. 엑셉트 성공 \n");

		//------------------------------
		// 세션 생성 및 세션 자료구조 등록
		//------------------------------

		// ID 할당
		SESSION_ID session_id = Get_SessionID();
		if (session_id == INVALID_SESSION_ID) {
			closesocket(accept_sock);
			continue;
		}

		//printf("2. 세션 아이디 할당 성공 Session id(%llu), index(%u), unique(%u) \n", session_id.session_id, session_id.session_index, session_id.session_unique);

		// 세션 설정
		Session& accept_session = session_array[session_id.session_index];
		accept_session.Clear();
		accept_session.Set(accept_sock, accept_ip, accept_port, session_id);
		auto ret = LanServer::Bind_IOCP(accept_sock, (ULONG_PTR)&accept_session);
		
		// 로그인 패킷 Send ( I/O Count 0 방지 )
		accept_session.io_count++;
		OnClientJoin(session_id.session_id);

		// 디버깅
		InterlockedIncrement(&session_count);
		//printf("[Accept] socket(%llu) \n", accept_sock);

		InterlockedIncrement(&accept_tps);
		//------------------------------
		// WSARecv
		//------------------------------
		ret = Post_Recv(&accept_session);

		// 방지용 I/O Count 차감
		if (InterlockedDecrement((LONG*)&accept_session.io_count) == 0) 
			Release_Session(&accept_session);
	}

	printf("End Accept Thread \n");
}

void LanServer::Release_Session(Session* p_session){
	for (int i = 0; i < p_session->sendPacket_count; i++)
		J_LIB::PacketBuffer::Free(p_session->sendPacket_array[i]);

	closesocket(p_session->sock);

	index_lock.lock();
	sessionIndex_stack.push(p_session->session_id.session_index);
	index_lock.unlock();
}

void LanServer::Send_Completion(Session* p_session){
	// Send 완료된 Packet 반환 및 send flag off
	for (int i = 0; i < p_session->sendPacket_count; i++)
		J_LIB::PacketBuffer::Free(p_session->sendPacket_array[i]);
	p_session->sendPacket_count = 0;
	if (InterlockedExchange8((char*)&p_session->send_flag, false) == false)
		throw;

	if (8 > p_session->send_buf.Get_UseSize())
		return;
	
	Post_Send(p_session);
}

// WSARecv 성공 여부 반환
bool LanServer::Post_Recv(Session* p_session){
	InterlockedIncrement((LONG*)&p_session->io_count);

	auto ret = IOCP_Recv(p_session);
	if (SOCKET_ERROR == ret) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			//printf("[리시브 실패] socket(%llu) \n", p_session->sock);
			if (InterlockedDecrement((LONG*)&p_session->io_count) == 0) {
				Release_Session(p_session);
			}
			return false;
		}
		else {
			//printf("Recv IO_Pending \n");
		}
	}

	//printf("[리시브 걸어놨다] socket(%llu) \n", p_session->sock);
	return true;
}

// SendQ Enqueue 만을 보장
bool LanServer::Send_Packet(SESSION_ID session_id, J_LIB::PacketBuffer* send_packet) {
	Session& session = session_array[session_id.session_index];

	// 패킷 완성 (페이로드 부 앞에 헤더 삽입)
	LAN_HEADER lan_header;
	lan_header.len = send_packet->Get_UseSize();
	send_packet->Set_header(&lan_header);
	PLONGLONG p_send_packet = (PLONGLONG)send_packet;

	// 네트워크 패킷, 컨텐츠 페킷 Enqueue (Enqueue 패킷 포인터, 8)
	send_packet->Increment_refCount();
	auto size = session.send_buf.Enqueue(&p_send_packet, sizeof(p_send_packet));
	//if (size != 8) {
	//	throw;
	//}

	Post_Send(&session);
	return false;
}


// 한 세션에 대해서 한 스레드에서만 들어갈 수 있음
bool LanServer::Post_Send(Session* p_session) {
	// Send Q 데이터 체크
	if (8 > p_session->send_buf.Get_UseSize())
		return false;

	if (InterlockedExchange8((char*)&p_session->send_flag, true) == true)
		return false;

	// Send Q 데이터 체크
	if (8 > p_session->send_buf.Get_UseSize()) {
		InterlockedExchange8((char*)&p_session->send_flag, false);
		return Post_Send(p_session);
	}

	InterlockedIncrement((LONG*)&p_session->io_count);
	auto ret = IOCP_Send(p_session);

	if (SOCKET_ERROR == ret) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			if (InterlockedDecrement((LONG*)&p_session->io_count) == 0) 
				Release_Session(p_session);

			//printf("[센드 실패] socket(%u) \n", p_session->sock);
			SocketError_Handling(p_session, IO_TYPE::SEND);
			return false;
		}
	}

	//printf("[오버랩 센드 했다] socket(%d) \n", p_session->sock);
	return true;
}

// Send RingBuffer 데이터 전부 Send
int LanServer::IOCP_Send(Session* p_session) {
	// Set var
	WSABUF wsaBuf[MAX_SEND_MSG];
	PacketBuffer* packet;

	for (int i = 0; i < MAX_SEND_MSG; i++) {
		//// 디버깅
		//auto size = p_session->send_buf.Get_UseSize();
		//if (size % 8 != 0) {
		//	printf("sendPacket_count : %d, i : %d", p_session->sendPacket_count);
		//	throw;
		//}

		if (8 > p_session->send_buf.Get_UseSize()) {
			if (i <= 0) {
				throw;
			}
			break;
		}

		p_session->send_buf.Dequeue(&packet, sizeof(packet));
		wsaBuf[i].buf = packet->Get_Packet();
		wsaBuf[i].len = packet->Get_PacketSize();

		p_session->sendPacket_array[p_session->sendPacket_count] = packet;
		p_session->sendPacket_count++;

		// 모니터링
		InterlockedIncrement(&sendMsg_tps);
	}

	return WSASend(p_session->sock, wsaBuf, p_session->sendPacket_count, NULL, 0, &p_session->send_overlapped, NULL);
}

// Recv RingBuffer에 Recv
int LanServer::IOCP_Recv(Session* p_session) {
	WSABUF wsaBuf[2];

	// Recv Remain Pos
	wsaBuf[1].buf = p_session->recv_buf.Get_BeginPos();
	wsaBuf[1].len = p_session->recv_buf.Remain_EnqueueSize();

	// Recv Write Pos
	wsaBuf[0].buf = p_session->recv_buf.Get_WritePos();
	wsaBuf[0].len = p_session->recv_buf.Direct_EnqueueSize();

	//printf("[IOCP_Recv] socket(%u),  %d + %d \n", p_session->sock, wsaBuf[0].len, wsaBuf[1].len);

	DWORD flags = 0;
	return WSARecv(p_session->sock, wsaBuf, 2, NULL, &flags, &p_session->recv_overlapped, NULL);
}

int LanServer::SocketError_Handling(Session* p_session, IO_TYPE io_type){
	//printf("unique (%d) I/O(%d) Error no : %d / (send 1, recv 2, accept 3)", p_session->session_id.session_unique, io_type, WSAGetLastError());
	// ip, id, io type, error num 로깅
	return 0;
}

void LanServer::Recv_Completion(Session* p_session){
	//------------------------------
	// var set
	//------------------------------
	PacketBuffer* contents_packet = PacketBuffer::Alloc();
	LAN_HEADER network_header;

	//------------------------------
	// 네트워크 헤더 검사 및 페이로드 디큐잉
	//------------------------------

	// ** 현재 프로토콜은 헤더만 달랑오는 경우가 없음
	int recv_len = p_session->recv_buf.Get_UseSize();
	while (LAN_HEADER_SIZE < recv_len) {
		p_session->recv_buf.Peek(&network_header, LAN_HEADER_SIZE);
		//printf("%d \n", network_header.len);

		// 페이로드 데이터 부족
		if (recv_len < network_header.len + LAN_HEADER_SIZE) 
			break;
		
		// 모니터링
		InterlockedIncrement(&recvMsg_tps);
		//------------------------------
		// OnRecv 
		//------------------------------
		contents_packet->Clear();
		p_session->recv_buf.Move_Front(LAN_HEADER_SIZE);
		p_session->recv_buf.Dequeue(contents_packet->Get_writePos(), network_header.len);
		contents_packet->Move_Wp(network_header.len);
		OnRecv(p_session->session_id, contents_packet);

		// set recv len
		recv_len = p_session->recv_buf.Get_UseSize();
	}
	PacketBuffer::Free(contents_packet);

	//------------------------------
	// Post Recv (Recv 걸어두기)
	//------------------------------
	if (!Post_Recv(p_session)) return;

	//------------------------------
	// Release
	//------------------------------
	return;
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

void LanServer::Monitoring() {
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

	index_lock.lock();
	if (sessionIndex_stack.empty()) {
		index_lock.unlock();
		return INVALID_SESSION_ID;
	}

	SESSION_ID session_id(sessionIndex_stack.top(), unique++);
	sessionIndex_stack.pop();
	index_lock.unlock();

	return session_id;
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
	send_buf.Clear();
}

void Session::Set(SOCKET sock, in_addr ip, WORD port, SESSION_ID session_id){
	this->sock = sock;
	this->ip = ip;
	this->port = port;
	this->session_id = session_id;
}