#include <WS2tcpip.h>
#include <WinSock2.h>
#include <memory.h>
#include <timeapi.h>
#include <atomic>
#include <iostream>
#include "NetServer.h"
#include "protocol.h"
#include "../../00 lib_jy/MemoryLogger.h"
#include "../../00 lib_jy/Logger.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

#define MAX_PACKET_LEN 500

using namespace std;

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
	if (maxWorker < activeWorker) {
		LOG("NetServer", LOG_LEVEL_FATAL, "WORKER_NUM_ERROR");
		throw std::exception("WORKER_NUM_ERROR");
	} 
	if (1 < (BYTE)netType) {
		LOG("NetServer", LOG_LEVEL_FATAL, "NET_TYPE_ERROR");
		throw std::exception("NET_TYPE_ERROR");
	}

	//////////////////////////////
	// Set Server
	//////////////////////////////

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		LOG("NetServer", LOG_LEVEL_FATAL, "WSAStartup()_ERROR : %d", WSAGetLastError());
		throw std::exception("WSAStartup_ERROR");
	}

	listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listen_sock) {
		LOG("NetServer", LOG_LEVEL_FATAL, "WSASocket()_ERROR : %d", WSAGetLastError());
		throw std::exception("WSASocket_ERROR");
	}

	// Set nagle
	if (!nagle_flag) {
		int opt_val = TRUE;
		if (setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val)) != 0) {
			LOG("NetServer", LOG_LEVEL_FATAL, "SET_NAGLE_ERROR : %d", WSAGetLastError());
			throw std::exception("SET_NAGLE_ERROR");
		}
	}

	// Set Linger
	LINGER linger = { 1, 0 };
	if (setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof linger) != 0) {
		LOG("NetServer", LOG_LEVEL_FATAL, "SET_LINGER_ERROR : %d", WSAGetLastError());
		throw std::exception("SET_LINGER_ERROR");
	}

	// bind 
	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if (0 != bind(listen_sock, (SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN))) {
		LOG("NetServer", LOG_LEVEL_FATAL, "bind()_ERROR : %d", WSAGetLastError());
		throw std::exception("bind()_ERROR");
	}

	// Set Session
	session_array = new Session[max_session];
	for (int i = 0; i < max_session; i++)
		sessionIndex_stack.Push(max_session - (1 + i));

	// Create IOCP
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, activeWorker);
	if (h_iocp == INVALID_HANDLE_VALUE || h_iocp == NULL) {
		LOG("NetServer", LOG_LEVEL_FATAL, "CreateIoCompletionPort()_ERROR : %d", GetLastError());
		throw std::exception("CreateIoCompletionPort()_ERROR");
	}

	// Create IOCP Worker
	workerThread_Pool = new thread[maxWorker];
	for (int i = 0; i < maxWorker; i++) {
		workerThread_Pool[i] = thread([this] {WorkerFunc(); });
	}
}
NetServer::~NetServer() {}

void NetServer::StartUp() {
	// listen
	if (0 != listen(listen_sock, SOMAXCONN_HINT(65535))) {
		LOG("NetServer", LOG_LEVEL_FATAL, "listen()_ERROR : %d", WSAGetLastError());
		throw std::exception("listen()_ERROR");
	}

	// Create Thread
	acceptThread = thread([this] {AcceptFunc(); });
	if (timeOut_flag) {
		timeOutThread = thread([this] {TimeoutFunc(); });
	}

	// LOG
	LOG("NetServer", LOG_LEVEL_INFO, "Start NetServer");
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
			LOG("NetServer", LOG_LEVEL_WARN, "accept() Fail, Error code : %d", WSAGetLastError());
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
}

void NetServer::TimeoutFunc() {
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
	for (;;) {
		DWORD	ioSize = 0;
		Session* p_session = 0;
		LPOVERLAPPED p_overlapped = 0;

		BOOL ret_GQCS = GetQueuedCompletionStatus(h_iocp, &ioSize, (PULONG_PTR)&p_session, &p_overlapped, INFINITE);

		// ��Ŀ ������ ����
		if (ioSize == 0 && p_session == 0 && p_overlapped == 0) {
			PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
			return;
		}
		// FIN
		if (ioSize == 0) {
			if (&p_session->send_overlapped == p_overlapped)
				LOG("NetServer", LOG_LEVEL_FATAL, "Zero Byte Send !!");
			goto Decrement_IOCount;
		}
		// PQCS
		else if ((ULONG_PTR)p_overlapped < (ULONG_PTR)PQCS_TYPE::NONE) {
			switch ((PQCS_TYPE)(byte)p_overlapped) {
			case PQCS_TYPE::SEND_POST: {
				SendPost(p_session);
				goto Decrement_IOCount;
			}

			case PQCS_TYPE::RELEASE_SESSION: {
				ReleaseSession(p_session);
				continue;
			}

			default:
				LOG("NetServer", LOG_LEVEL_FATAL, "PQCS Default");
				break;
			}
		}

		// recv �Ϸ�����
		if (&p_session->recv_overlapped == p_overlapped) {
			if (ret_GQCS) {
				p_session->recv_buf.Move_Rear(ioSize);
				p_session->lastRecvTime = timeGetTime();
				// ���� ��� �ܺ� ��� ����
				if (NetType::LAN == netType) {
					RecvCompletionLan(p_session);
				}
				else {
					RecvCompletionNet(p_session);
				}
			}
			else {
				LOG("NetServer", LOG_LEVEL_DEBUG, "Overlapped Recv Fail");
			}
		}
		// send �Ϸ�����
		else if (&p_session->send_overlapped == p_overlapped) {
			if (ret_GQCS) {
				SendCompletion(p_session);
			}
			else {
				LOG("NetServer", LOG_LEVEL_DEBUG, "Overlapped Send Fail");
			}
		}
		else {
			LOG("NetServer", LOG_LEVEL_FATAL, "GQCS INVALID Overlapped!!");
		}

	Decrement_IOCount:
		DecrementIOCount(p_session);
	}
}

bool NetServer::ReleaseSession(Session* p_session) {
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
		for (int i = 0; i < p_session->sendPacketCount; i++) {
			PacketBuffer::Free(p_session->sendPacketArr[i]);
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

void NetServer::SendCompletion(Session* p_session) {
	// Send Packet Free
	for (int i = 0; i < p_session->sendPacketCount ; i++) {
		PacketBuffer::Free(p_session->sendPacketArr[i]);
	}
	InterlockedAdd((LONG*)&sendMsgTPS, p_session->sendPacketCount);
	p_session->sendPacketCount = 0;

	// Send Flag OFF
	InterlockedExchange8((char*)&p_session->send_flag, false);

	// Send ���� üũ
	if (p_session->disconnect_flag)	return;
	if (p_session->sendQ.GetUseCount() <= 0) return;
	SendPost(p_session);
}

// SendQ Enqueue, SendPost
void NetServer::SendPacket(SESSION_ID session_id, PacketBuffer* send_packet) {
	Session* p_session = ValidateSession(session_id);
	if (nullptr == p_session)
		return;

	if (p_session->disconnect_flag) {
		DecrementIOCountPQCS(p_session);
		return;
	}

	// LAN, NET ����
	if (NetType::LAN == netType) {
		send_packet->SetLanHeader();
		send_packet->IncrementRefCount();
	}
	else {
		send_packet->SetNetHeader(protocol_code, private_key);
		send_packet->IncrementRefCount();
	}

	p_session->sendQ.Enqueue(send_packet);
#if PQCS_SEND
	if (p_session->send_flag == false) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)p_session, (LPOVERLAPPED)PQCS_TYPE::SEND_POST);
	}
	else {
		DecrementIOCountPQCS(p_session);
	}
#else
	AsyncSend(p_session);
	DecrementIOCountPQCS(p_session);
#endif
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

	// MAX SEND ���� �ʰ�
	if (MAX_SEND_MSG < p_session->sendQ.GetUseCount()) {
		DisconnectSession(p_session);
		return false;
	}
	if (NetType::LAN == netType) {
		for (int i = 0; (i < MAX_SEND_MSG && 0 < p_session->sendQ.GetUseCount()); i++) {
			p_session->sendQ.Dequeue((PacketBuffer**)&p_session->sendPacketArr[i]);
			p_session->sendPacketCount++;
			wsaBuf[i].buf = p_session->sendPacketArr[i]->GetLanPacketPos();
			wsaBuf[i].len = p_session->sendPacketArr[i]->GetLanPacketSize();
		}
	}
	else {
		for (int i = 0; (i < MAX_SEND_MSG && 0 < p_session->sendQ.GetUseCount()); i++) {
			p_session->sendQ.Dequeue((PacketBuffer**)&p_session->sendPacketArr[i]);
			p_session->sendPacketCount++;
			wsaBuf[i].buf = p_session->sendPacketArr[i]->GetNetPacketPos();
			wsaBuf[i].len = p_session->sendPacketArr[i]->GetNetPacketSize();
		}
	}

	IncrementIOCount(p_session);
	ZeroMemory(&p_session->send_overlapped, sizeof(p_session->send_overlapped));
	if (SOCKET_ERROR == WSASend(p_session->sock, wsaBuf, p_session->sendPacketCount , NULL, 0, &p_session->send_overlapped, NULL)) {
		const auto err_no = WSAGetLastError();
		if (ERROR_IO_PENDING != err_no) { // Send ����
			LOG("NetServer", LOG_LEVEL_DEBUG, "WSASend() Fail, Error code : %d", WSAGetLastError());
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
			LOG("NetServer", LOG_LEVEL_DEBUG, "WSARecv() Fail, Error code : %d", WSAGetLastError());
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

void NetServer::RecvCompletionLan(Session* p_session) {
	// ��Ŷ ����
	for (;;) {
		int recv_len = p_session->recv_buf.GetUseSize();
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
		p_session->recv_buf.Dequeue(contents_packet->GetWritePos(), lanHeader.len);
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

void NetServer::RecvCompletionNet(Session* p_session) {
	// ��Ŷ ����
	for (;;) {
		int recv_len = p_session->recv_buf.GetUseSize();
		if (recv_len < NET_HEADER_SIZE)
			break;

		// ��� ī��
		char encryptPacket[200];
		p_session->recv_buf.Peek(encryptPacket, NET_HEADER_SIZE);

		// code �˻�
		BYTE code = ((NET_HEADER*)encryptPacket)->code;
		if (code != protocol_code) {
			LOG("NetServer", LOG_LEVEL_WARN, "Recv Packet is wrong code!!", WSAGetLastError());
			DisconnectSession(p_session);
			break;
		}

		// ���̷ε� ������ ����
		WORD payload_len = ((NET_HEADER*)encryptPacket)->len;
		if (recv_len < (NET_HEADER_SIZE + payload_len)) {
			break;
		}

		// Recv Data ��Ŷ ȭ
		p_session->recv_buf.Move_Front(NET_HEADER_SIZE);
		p_session->recv_buf.Dequeue(encryptPacket + NET_HEADER_SIZE, payload_len);

		// ��ȣ��Ŷ ����
		PacketBuffer* decrypt_packet = PacketBuffer::Alloc();
		if (!decrypt_packet->DecryptPacket(encryptPacket, private_key)) {
			PacketBuffer::Free(decrypt_packet);
			LOG("NetServer", LOG_LEVEL_WARN, "Recv Packet is wrong checksum!!", WSAGetLastError());
			DisconnectSession(p_session);
			break;
		}

		// ����� ��Ŷ ó��
		OnRecv(p_session->session_id, decrypt_packet);
		InterlockedIncrement(&recvMsgTPS);

		// ��ȣ��Ŷ, ��ȣȭ ��Ŷ Free
		PacketBuffer::Free(decrypt_packet);
	}

	//------------------------------
	// Post Recv
	//------------------------------
	if (!p_session->disconnect_flag) {
		AsyncRecv(p_session);
	}
}

Session* NetServer::ValidateSession(SESSION_ID session_id) {
	Session* p_session = &session_array[session_id.session_index];
	IncrementIOCount(p_session);

	// ���� ������ ����
	if (true == p_session->release_flag) {
		DecrementIOCountPQCS(p_session);
		return nullptr;
	}
	// ���� ����
	if (p_session->session_id != session_id) {
		DecrementIOCountPQCS(p_session);
		return nullptr;
	}

	// ���� ���ɼ� ����
	return p_session;
}

bool NetServer::Disconnect(SESSION_ID session_id) {
	Session* p_session = ValidateSession(session_id);
	if (nullptr == p_session) return true;
	DisconnectSession(p_session);
	DecrementIOCountPQCS(p_session);
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