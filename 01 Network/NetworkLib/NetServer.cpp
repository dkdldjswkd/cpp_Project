#include <WS2tcpip.h>
#include <WinSock2.h>
#include <memory.h>
#include <timeapi.h>
#include "NetServer.h"
#include "protocol.h"
#include "MemoryLogger.h"
#include "Logger.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

using namespace std;
using namespace J_LIB;

//------------------------------
// Server Func
//------------------------------
NetServer::NetServer(const char* systemFile, const char* server) {
	// Read SystemFile
	parser.LoadFile(systemFile);
	parser.GetValue(server, "PROTOCOL_CODE", (int*)&protocol_code);
	parser.GetValue(server, "PRIVATE_KEY", (int*)&private_key);
	parser.GetValue(server, "NET_TYPE", (int*)&netType);
	parser.GetValue(server, "PORT", (int*)&server_port);
	parser.GetValue(server, "MAX_SESSION", (int*)&max_session);
	parser.GetValue(server, "NAGLE", (int*)&nagle_flag);
	parser.GetValue(server, "TIME_OUT_FLAG", (int*)&timeOut_flag);
	parser.GetValue(server, "TIME_OUT", (int*)&timeOut);
	parser.GetValue(server, "TIME_OUT_CYCLE", (int*)&timeOutCycle);
	parser.GetValue(server, "MAX_WORKER", (int*)&maxWorker);
	parser.GetValue(server, "ACTIVE_WORKER", (int*)&activeWorker);

	// Check system
	if (maxWorker < activeWorker) CRASH();
	if (1 < (BYTE)netType) CRASH();

	//////////////////////////////
	// Set Server
	//////////////////////////////

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		CRASH();

	listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) CRASH();

	// Set nagle
	if (!nagle_flag) {
		int opt_val = TRUE;
		setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));
	}

	// Set Linger
	LINGER linger = { 1, 0 };
	setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof linger);

	// bind 
	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if (0 != bind(listen_sock, (SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN))) CRASH();

	// Set Session
	session_array = new Session[max_session];
	for (int i = 0; i < max_session; i++)
		sessionIndex_stack.Push(max_session - (1 + i));

	// Create IOCP
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, activeWorker);
	if (INVALID_HANDLE_VALUE == h_iocp) CRASH();

	// Create IOCP Worker
	workerThread_Pool = new thread[maxWorker];
	for (int i = 0; i < maxWorker; i++) {
		workerThread_Pool[i] = thread([this] {WorkerFunc(); });
	}
}
NetServer::~NetServer() {}

void NetServer::StartUp() {
	// listen
	if (0 != listen(listen_sock, SOMAXCONN_HINT(65535))) CRASH();

	// Create Thread
	acceptThread = thread([this] {AcceptFunc(); });
	if (timeOut_flag) {
		timeOutThread = thread([this] {TimeOutFunc(); });
	}

	// LOG
	LOG("NetworkLib", LOG_LEVEL_DEBUG, "Start Server !");
}

void NetServer::AcceptFunc() {
	printf("Start Accept Thread \n");
	for (;;) {
		sockaddr_in client_addr;
		int addr_len = sizeof(client_addr);

		//------------------------------
		// Accept
		//------------------------------
		auto accept_sock = accept(listen_sock, (sockaddr*)&client_addr, &addr_len);
		acceptTotal++;
		if (accept_sock == INVALID_SOCKET) {
			LOG("NetworkLib", LOG_LEVEL_DEBUG, "accept() Fail, Error code : %d", WSAGetLastError());
			continue;
		}
		if (max_session <= sessionCount) {
			closesocket(accept_sock);
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
		acceptTPS++;

		// Bind IOCP
		CreateIoCompletionPort((HANDLE)accept_sock, h_iocp, (ULONG_PTR)p_accept_session, 0);

		//------------------------------
		// ���� �α��� �� �۾�
		//------------------------------
		OnClientJoin(session_id.session_id);
		InterlockedIncrement((LONG*)&sessionCount);

		//------------------------------
		// WSARecv
		//------------------------------
		AsyncRecv(p_accept_session);

		// ���� I/O Count ����
		DecrementIOCount(p_accept_session);
	}
	printf("End Accept Thread \n");
}

void NetServer::TimeOutFunc() {
	for (;;) {
		Sleep(timeOutCycle);
		INT64 cur_time = timeGetTime();
		for (int i = 0; i < max_session; i++) {
			Session* p_session = &session_array[i];
			SESSION_ID id = p_session->session_id;

			// Ÿ�Ӿƿ� ó�� ��� �ƴ� (Interlocked ���� �ּ�ȭ�ϱ� ����)
			if (session_array[i].release_flag || (timeOut > cur_time - session_array[i].lastRecvTime))
				continue;

			// Ÿ�Ӿƿ� ó�� ����� �� ����
			IncrementIOCount(p_session);
			// ������ �������� �Ǵ�.
			if (true == p_session->release_flag) {
				DecrementIOCount(p_session);
				continue;
			}
			// Ÿ�Ӿƿ� ���� �Ǵ�.
			if (timeOut > cur_time - session_array[i].lastRecvTime) {
				DecrementIOCount(p_session);
				continue;
			}

			// Ÿ�Ӿƿ� ó��
			DisconnectSession(p_session);
			DecrementIOCount(p_session);
		}
	}
}

void NetServer::WorkerFunc() {
	printf("Start Worker Thread \n");
	for (;;) {
		DWORD	io_size = 0;
		Session* p_session = 0;
		LPOVERLAPPED p_overlapped = 0;

		BOOL ret_GQCS = GetQueuedCompletionStatus(h_iocp, &io_size, (PULONG_PTR)&p_session, &p_overlapped, INFINITE);

		// ��Ŀ ������ ����
		if (io_size == 0 && p_session == 0 && p_overlapped == 0) {
			PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
			return;
		}
		// FIN
		if (io_size == 0) {
			if (&p_session->send_overlapped == p_overlapped)
				LOG("NetworkLib", LOG_LEVEL_FATAL, "Zero Byte Send !!");
			goto Decrement_IOCount;
		}
		// PQCS
		if ((PQCS_TYPE)((byte)p_overlapped) <= PQCS_TYPE::DECREMENT_IO) {
			switch ((PQCS_TYPE)((byte)p_overlapped)) {
				case PQCS_TYPE::SEND_POST:
					SendPost(p_session);
					goto Decrement_IOCount;

				case PQCS_TYPE::DECREMENT_IO:
					goto Decrement_IOCount;

				default:
					LOG("NetworkLib", LOG_LEVEL_FATAL, "PQCS Default");
					break;
			}
		}

		// recv �Ϸ�����
		if (&p_session->recv_overlapped == p_overlapped) {
			if (ret_GQCS) {
				p_session->recv_buf.Move_Rear(io_size);
				p_session->lastRecvTime = timeGetTime();
				// ���� ��� �ܺ� ��� ����
				if (NetType::LAN == netType) {
					RecvCompletion_LAN(p_session);
				}
				else {
					RecvCompletion_NET(p_session);
				}
			}  
			else {
				LOG("NetworkLib", LOG_LEVEL_DEBUG, "Overlapped Recv Fail");
			}
		}
		// send �Ϸ�����
		else if (&p_session->send_overlapped == p_overlapped) {
			if (ret_GQCS) {
				SendCompletion(p_session);
			}
			else {
				LOG("NetworkLib", LOG_LEVEL_DEBUG, "Overlapped Send Fail");
			}
		}
		else {
			LOG("NetworkLib", LOG_LEVEL_FATAL, "GQCS INVALID Overlapped!!");
		}

	Decrement_IOCount:
		DecrementIOCount(p_session);
	}
	printf("End Worker Thread \n");
}

bool NetServer::ReleaseSession(Session* p_session){
	if (0 != p_session->io_count)
		return false;

	// * release_flag(0), iocount(0) -> release_flag(1), iocount(0)
	if (0 == InterlockedCompareExchange64((long long*)&p_session->release_flag, 1, 0)) {
		// ���ҽ� ���� (����, ��Ŷ)
		closesocket(p_session->sock);
		PacketBuffer* packet;
		while (p_session->sendQ.Dequeue(&packet)) {
			PacketBuffer::Free(packet);
		}
		for (int i = 0; i < p_session->sendPacket_count; i++) {
			PacketBuffer::Free(p_session->sendPacket_array[i]);
		}

		// ����� ���ҽ� ����
		OnClientLeave(p_session->session_id);

		// ���� ��ȯ
		sessionIndex_stack.Push(p_session->session_id.session_index);
		InterlockedDecrement((LONG*)&sessionCount);
		return true;
	}
	return false;
}

void NetServer::SendCompletion(Session* p_session){
	// Send Packet Free
	for (int i = 0; i < p_session->sendPacket_count; i++) {
		PacketBuffer::Free(p_session->sendPacket_array[i]);
		InterlockedIncrement(&sendMsgTPS);
	}
	p_session->sendPacket_count = 0;

	// Send Flag OFF
	InterlockedExchange8((char*)&p_session->send_flag, false);

	// Send ���� üũ
	if (p_session->disconnect_flag)	return;
	if (p_session->sendQ.GetUseCount() <= 0) return;
	SendPost(p_session);
}

// SendQ Enqueue, SendPost
void NetServer::SendPacket(SESSION_ID session_id, PacketBuffer* send_packet) {
	Session* p_session = Check_InvalidSession(session_id);
	if (nullptr == p_session)
		return;

	if (p_session->disconnect_flag) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)p_session, (LPOVERLAPPED)PQCS_TYPE::DECREMENT_IO);
		return;
	}

	// LAN, NET ����
	if (NetType::LAN == netType) {
		send_packet->Set_LanHeader();
		send_packet->Increment_refCount();
	}
	else {
		send_packet->Set_NetHeader(protocol_code, private_key);
		send_packet->Increment_refCount();
	}

	p_session->sendQ.Enqueue(send_packet);
	PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)p_session, (LPOVERLAPPED)PQCS_TYPE::SEND_POST);
}

// AsyncSend Call �õ�
bool NetServer::SendPost(Session* p_session) {
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

// WSASend() call
int NetServer::AsyncSend(Session* p_session) {
	WSABUF wsaBuf[MAX_SEND_MSG];

	if (NetType::LAN == netType) {
		for (int i = 0; i < MAX_SEND_MSG; i++) {
			if (p_session->sendQ.GetUseCount() <= 0) {
				p_session->sendPacket_count = i;
				break;
			}
			p_session->sendQ.Dequeue((PacketBuffer**)&p_session->sendPacket_array[i]);
			wsaBuf[i].buf = p_session->sendPacket_array[i]->Get_PacketPos_LAN();
			wsaBuf[i].len = p_session->sendPacket_array[i]->Get_PacketSize_LAN();
		}
	}
	else {
		for (int i = 0; i < MAX_SEND_MSG; i++) {
			if (p_session->sendQ.GetUseCount() <= 0) {
				p_session->sendPacket_count = i;
				break;
			}
			p_session->sendQ.Dequeue((PacketBuffer**)&p_session->sendPacket_array[i]);
			wsaBuf[i].buf = p_session->sendPacket_array[i]->Get_PacketPos_NET();
			wsaBuf[i].len = p_session->sendPacket_array[i]->Get_PacketSize_NET();
		}
	}
	// MAX SEND ���� �ʰ�
	if (p_session->sendPacket_count == 0) {
		p_session->sendPacket_count = MAX_SEND_MSG;
		DisconnectSession(p_session);
		return false;
	}

	IncrementIOCount(p_session);
	ZeroMemory(&p_session->send_overlapped, sizeof(p_session->send_overlapped));
	if (SOCKET_ERROR == WSASend(p_session->sock, wsaBuf, p_session->sendPacket_count, NULL, 0, &p_session->send_overlapped, NULL)) {
		const auto err_no = WSAGetLastError();
		if (ERROR_IO_PENDING != err_no) { // Send ����
			LOG("NetworkLib", LOG_LEVEL_DEBUG, "WSASend() Fail, Error code : %d", WSAGetLastError());
			DisconnectSession(p_session);
			DecrementIOCount(p_session);
			return false;
		}
	}
	return true;
}

bool NetServer::AsyncRecv(Session* p_session) {
	DWORD flags = 0;
	WSABUF wsaBuf[2];

	// Recv Write Pos
	wsaBuf[0].buf = p_session->recv_buf.Get_WritePos();
	wsaBuf[0].len = p_session->recv_buf.Direct_EnqueueSize();
	// Recv Remain Pos
	wsaBuf[1].buf = p_session->recv_buf.Get_BeginPos();
	wsaBuf[1].len = p_session->recv_buf.Remain_EnqueueSize();

	IncrementIOCount(p_session);
	ZeroMemory(&p_session->recv_overlapped, sizeof(p_session->recv_overlapped));
	if (SOCKET_ERROR == WSARecv(p_session->sock, wsaBuf, 2, NULL, &flags, &p_session->recv_overlapped, NULL)) {
		if (WSAGetLastError() != ERROR_IO_PENDING) { // Recv ����
			LOG("NetworkLib", LOG_LEVEL_DEBUG, "WSARecv() Fail, Error code : %d", WSAGetLastError());
			DecrementIOCount(p_session);
			return false;
		}
	}

	// Disconnect üũ
	if (p_session->disconnect_flag) {
		CancelIoEx((HANDLE)p_session->sock, NULL);
		return false;
	}
	return true;
}

void NetServer::RecvCompletion_LAN(Session* p_session){
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
		PacketBuffer* contents_packet = PacketBuffer::Alloc();

		// ������ ��Ŷ ����
		p_session->recv_buf.Move_Front(LAN_HEADER_SIZE);
		p_session->recv_buf.Dequeue(contents_packet->Get_writePos(), lanHeader.len);
		contents_packet->Move_Wp(lanHeader.len);

		// ����� ��Ŷ ó��
		OnRecv(p_session->session_id, contents_packet);
		InterlockedIncrement(&recvMsgTPS);

		auto ret = PacketBuffer::Free(contents_packet);
	}

	//------------------------------
	// Post Recv (Recv �ɾ�α�)
	//------------------------------
	if (false == p_session->disconnect_flag) {
		AsyncRecv(p_session);
	}
}

void NetServer::RecvCompletion_NET(Session* p_session){
	// ��Ŷ ����
	for (;;) {
		int recv_len = p_session->recv_buf.Get_UseSize();
		if (recv_len < NET_HEADER_SIZE)
			break;

		// ��� ī��
		PacketBuffer* encrypt_packet = PacketBuffer::Alloc();
		char* p_packet = encrypt_packet->Get_PacketPos_NET();
		p_session->recv_buf.Peek(p_packet, NET_HEADER_SIZE);

		BYTE code = ((NET_HEADER*)p_packet)->code;
		WORD payload_len = ((NET_HEADER*)p_packet)->len;

		// code �˻�
		if (code != protocol_code) {
			PacketBuffer::Free(encrypt_packet);
			LOG("NetworkLib", LOG_LEVEL_WARN, "Recv Packet is wrong code!!", WSAGetLastError());
			DisconnectSession(p_session);
			break;
		}

		// ���̷ε� ������ ����
		if (recv_len < (NET_HEADER_SIZE + payload_len)) {
			PacketBuffer::Free(encrypt_packet);
			break;
		}

		// Recv Data ��Ŷ ȭ
		p_session->recv_buf.Move_Front(NET_HEADER_SIZE);
		p_session->recv_buf.Dequeue(encrypt_packet->Get_writePos(), payload_len);
		encrypt_packet->Move_Wp(payload_len);

		// ��ȣ��Ŷ ����
		PacketBuffer* decrypt_packet = PacketBuffer::Alloc();
		if (!decrypt_packet->DecryptPacket(encrypt_packet, private_key)) {
			PacketBuffer::Free(encrypt_packet);
			PacketBuffer::Free(decrypt_packet);
			LOG("NetworkLib", LOG_LEVEL_WARN, "Recv Packet is wrong checksum!!", WSAGetLastError());
			DisconnectSession(p_session);
			break;
		}

		// ����� ��Ŷ ó��
		OnRecv(p_session->session_id, decrypt_packet);
		InterlockedIncrement(&recvMsgTPS);

		// ��ȣ��Ŷ, ��ȣȭ ��Ŷ Free
		PacketBuffer::Free(encrypt_packet);
		PacketBuffer::Free(decrypt_packet);
	}

	//------------------------------
	// Post Recv
	//------------------------------
	if (!p_session->disconnect_flag) {
		AsyncRecv(p_session);
	}
}

Session* NetServer::Check_InvalidSession(SESSION_ID session_id) {
	Session* p_session = &session_array[session_id.session_index];
	IncrementIOCount(p_session);

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

bool NetServer::Disconnect(SESSION_ID session_id) {
	Session* p_session = Check_InvalidSession(session_id);
	if (nullptr == p_session) return true;
	DisconnectSession(p_session);
	PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)p_session, (LPOVERLAPPED)PQCS_TYPE::DECREMENT_IO);
	return true;
}

void NetServer::CleanUp() {
	// AcceptThread ����
	if (acceptThread.joinable()) {
		acceptThread.join();
	}

	// WorkerThread ����
	//PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
	for (int i = 0; i < maxWorker; i++) {
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

//------------------------------
// SESSION_ID
//------------------------------

SESSION_ID NetServer::Get_SessionID() {
	static DWORD unique = 1;

	DWORD index;
	if (false == sessionIndex_stack.Pop(&index)) {
		return INVALID_SESSION_ID;
	}

	SESSION_ID session_id(index, unique++);
	return session_id;
}