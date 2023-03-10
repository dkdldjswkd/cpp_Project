#include <WS2tcpip.h>
#include <WinSock2.h>
#include <memory.h>
#include <timeapi.h>
#include "NetClient.h"
#include "protocol.h"
#include "../../00 lib_jy/MemoryLogger.h"
#include "../../00 lib_jy/Logger.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")
//#pragma comment(lib, "../../00 lib_jy/lib_jy.lib")

using namespace std;

//------------------------------
// Server Func
//------------------------------
NetClient::NetClient(const char* systemFile, const char* server) {
	// Read SystemFile
	parser.LoadFile(systemFile);
	parser.GetValue(server, "PROTOCOL_CODE", (int*)&protocol_code);
	parser.GetValue(server, "PRIVATE_KEY", (int*)&private_key);
	parser.GetValue(server, "NET_TYPE", (int*)&netType);
	parser.GetValue(server, "IP", server_ip);
	parser.GetValue(server, "PORT", (int*)&server_port);
	parser.GetValue(server, "NAGLE", (int*)&nagle_flag);

	// Check system
	if (1 < (BYTE)netType) CRASH();

	//////////////////////////////
	// Set Client
	//////////////////////////////

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		CRASH();

	client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == client_sock) CRASH();

	// Set nagle
	if (!nagle_flag) {
		int opt_val = TRUE;
		setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));
	}

	// Set Linger
	LINGER linger = { 1, 0 };
	setsockopt(client_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof linger);

	// Create IOCP
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 2);
	if (INVALID_HANDLE_VALUE == h_iocp) CRASH();

	// BIND IOCP
	CreateIoCompletionPort((HANDLE)client_sock, h_iocp, (ULONG_PTR)&client_session, 0);

	// Create IOCP Worker Thread
	workerThread = thread([this] { WorkerFunc(); });
}
NetClient::~NetClient() {}

void NetClient::StartUp() {
	// Set server address
	sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) != 1) {
		CRASH();
	}

	// Set client
	client_session.Set(client_sock, server_address.sin_addr, server_port, 0);

	// Connect Fail
	if (connect(client_sock, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
		LOG("NetClient", LOG_LEVEL_WARN, "Start Client Connect FAIL");
		// Connect 시도 스레드 생성?
	}
	// Connect Success
	else {
		OnConnect();
		AsyncRecv();
		LOG("NetClient", LOG_LEVEL_DEBUG, "Start Client Connect Success");
	}

	// 생성 I/O Count 차감
	DecrementIOCount();
	LOG("NetClient", LOG_LEVEL_DEBUG, "Start Client !");
}

void NetClient::WorkerFunc() {
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
			if (&p_session->send_overlapped == p_overlapped)
				LOG("NetClient", LOG_LEVEL_FATAL, "Zero Byte Send !!");
			goto Decrement_IOCount;
		}
		// PQCS
		else if ((PQCS_TYPE)((byte)p_overlapped) <= PQCS_TYPE::DECREMENT_IO) {
			switch ((PQCS_TYPE)((byte)p_overlapped)) {
				case PQCS_TYPE::SEND_POST:
					SendPost();
					goto Decrement_IOCount;

				case PQCS_TYPE::DECREMENT_IO:
					goto Decrement_IOCount;

				default:
					LOG("NetClient", LOG_LEVEL_FATAL, "PQCS Default");
					break;
			}
		}

		// recv 완료통지
		if (&p_session->recv_overlapped == p_overlapped) {
			if (ret_GQCS) {
				p_session->recv_buf.Move_Rear(io_size);
				p_session->lastRecvTime = timeGetTime();
				// 내부 통신 외부 통신 구분
				if (NetType::LAN == netType) {
					RecvCompletion_LAN();
				}
				else {
					RecvCompletion_NET();
				}
			}  
			else {
				LOG("NetClient", LOG_LEVEL_DEBUG, "Overlapped Recv Fail");
			}
		}
		// send 완료통지
		else if (&p_session->send_overlapped == p_overlapped) {
			if (ret_GQCS) {
				SendCompletion();
			}
			else {
				LOG("NetClient", LOG_LEVEL_DEBUG, "Overlapped Send Fail");
			}
		}
		else {
			LOG("NetClient", LOG_LEVEL_FATAL, "GQCS INVALID Overlapped!!");
		}

	Decrement_IOCount:
		DecrementIOCount();
	}
}

bool NetClient::ReleaseSession(){
	if (0 != client_session.io_count)
		return false;

	// * release_flag(0), iocount(0) -> release_flag(1), iocount(0)
	if (0 == InterlockedCompareExchange64((long long*)&client_session.release_flag, 1, 0)) {
		// 리소스 정리 (소켓, 패킷)
		closesocket(client_session.sock);
		PacketBuffer* packet;
		while (client_session.sendQ.Dequeue(&packet)) {
			PacketBuffer::Free(packet);
		}
		for (int i = 0; i < client_session.sendPacketCount; i++) {
			PacketBuffer::Free(client_session.sendPacketArr[i]);
		}

		// 사용자 리소스 정리
		OnDisconnect();
		return true;
	}
	return false;
}

void NetClient::SendCompletion(){
	// Send Packet Free
	for (int i = 0; i < client_session.sendPacketCount; i++) {
		PacketBuffer::Free(client_session.sendPacketArr[i]);
		InterlockedIncrement(&sendMsgTPS);
	}
	client_session.sendPacketCount = 0;

	// Send Flag OFF
	InterlockedExchange8((char*)&client_session.send_flag, false);

	// Send 조건 체크
	if (client_session.disconnect_flag)	return;
	if (client_session.sendQ.GetUseCount() <= 0) return;
	SendPost();
}

// SendQ Enqueue, SendPost
void NetClient::SendPacket(PacketBuffer* send_packet) {
	if (false == Check_InvalidSession())
		return;

	if (client_session.disconnect_flag) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)&client_session, (LPOVERLAPPED)PQCS_TYPE::DECREMENT_IO);
		return;
	}

	// LAN, NET 구분
	if (NetType::LAN == netType) {
		send_packet->Set_LanHeader();
		send_packet->Increment_refCount();
	}
	else {
		send_packet->Set_NetHeader(protocol_code, private_key);
		send_packet->Increment_refCount();
	}

	client_session.sendQ.Enqueue(send_packet);
	PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)&client_session, (LPOVERLAPPED)PQCS_TYPE::SEND_POST);
}

// AsyncSend Call 시도
bool NetClient::SendPost() {
	// Empty return
	if (client_session.sendQ.GetUseCount() <= 0)
		return false;

	// Send 1회 체크 (send flag, true 시 send 진행 중)
	if (client_session.send_flag == true)
		return false;
	if (InterlockedExchange8((char*)&client_session.send_flag, true) == true)
		return false;

	// Empty continue
	if (client_session.sendQ.GetUseCount() <= 0) {
		InterlockedExchange8((char*)&client_session.send_flag, false);
		return SendPost();
	}

	AsyncSend();
	return true;
}

// WSASend() call
int NetClient::AsyncSend() {
	WSABUF wsaBuf[MAX_SEND_MSG];

	if (NetType::LAN == netType) {
		for (int i = 0; i < MAX_SEND_MSG; i++) {
			if (client_session.sendQ.GetUseCount() <= 0) {
				client_session.sendPacketCount = i;
				break;
			}
			client_session.sendQ.Dequeue((PacketBuffer**)&client_session.sendPacketArr[i]);
			wsaBuf[i].buf = client_session.sendPacketArr[i]->Get_PacketPos_LAN();
			wsaBuf[i].len = client_session.sendPacketArr[i]->Get_PacketSize_LAN();
		}
	}
	else {
		for (int i = 0; i < MAX_SEND_MSG; i++) {
			if (client_session.sendQ.GetUseCount() <= 0) {
				client_session.sendPacketCount = i;
				break;
			}
			client_session.sendQ.Dequeue((PacketBuffer**)&client_session.sendPacketArr[i]);
			wsaBuf[i].buf = client_session.sendPacketArr[i]->Get_PacketPos_NET();
			wsaBuf[i].len = client_session.sendPacketArr[i]->Get_PacketSize_NET();
		}
	}
	// MAX SEND 제한 초과
	if (client_session.sendPacketCount == 0) {
		client_session.sendPacketCount = MAX_SEND_MSG;
		DisconnectSession();
		return false;
	}

	IncrementIOCount();
	ZeroMemory(&client_session.send_overlapped, sizeof(client_session.send_overlapped));
	if (SOCKET_ERROR == WSASend(client_session.sock, wsaBuf, client_session.sendPacketCount, NULL, 0, &client_session.send_overlapped, NULL)) {
		const auto err_no = WSAGetLastError();
		if (ERROR_IO_PENDING != err_no) { // Send 실패
			LOG("NetClient", LOG_LEVEL_DEBUG, "WSASend() Fail, Error code : %d", WSAGetLastError());
			DisconnectSession();
			DecrementIOCount();
			return false;
		}
	}
	return true;
}

bool NetClient::AsyncRecv() {
	DWORD flags = 0;
	WSABUF wsaBuf[2];

	// Recv Write Pos
	wsaBuf[0].buf = client_session.recv_buf.Get_WritePos();
	wsaBuf[0].len = client_session.recv_buf.Direct_EnqueueSize();
	// Recv Remain Pos
	wsaBuf[1].buf = client_session.recv_buf.Get_BeginPos();
	wsaBuf[1].len = client_session.recv_buf.Remain_EnqueueSize();

	IncrementIOCount();
	ZeroMemory(&client_session.recv_overlapped, sizeof(client_session.recv_overlapped));
	if (SOCKET_ERROR == WSARecv(client_session.sock, wsaBuf, 2, NULL, &flags, &client_session.recv_overlapped, NULL)) {
		if (WSAGetLastError() != ERROR_IO_PENDING) { // Recv 실패
			LOG("NetClient", LOG_LEVEL_DEBUG, "WSARecv() Fail, Error code : %d", WSAGetLastError());
			DecrementIOCount();
			return false;
		}
	}

	// Disconnect 체크
	if (client_session.disconnect_flag) {
		CancelIoEx((HANDLE)client_session.sock, NULL);
		return false;
	}
	return true;
}

void NetClient::RecvCompletion_LAN(){
	// 패킷 조립
	for (;;) {
		int recv_len = client_session.recv_buf.GetUseSize();
		if (recv_len <= LAN_HEADER_SIZE)
			break;

		LAN_HEADER lanHeader;
		client_session.recv_buf.Peek(&lanHeader, LAN_HEADER_SIZE);

		// 페이로드 데이터 부족
		if (recv_len < lanHeader.len + LAN_HEADER_SIZE)
			break;

		//------------------------------
		// OnRecv (네트워크 헤더 제거)
		//------------------------------
		PacketBuffer* contents_packet = PacketBuffer::Alloc();

		// 컨텐츠 패킷 생성
		client_session.recv_buf.Move_Front(LAN_HEADER_SIZE);
		client_session.recv_buf.Dequeue(contents_packet->Get_writePos(), lanHeader.len);
		contents_packet->Move_Wp(lanHeader.len);

		// 사용자 패킷 처리
		OnRecv(contents_packet);
		InterlockedIncrement(&recvMsgTPS);

		auto ret = PacketBuffer::Free(contents_packet);
	}

	//------------------------------
	// Post Recv (Recv 걸어두기)
	//------------------------------
	if (false == client_session.disconnect_flag) {
		AsyncRecv();
	}
}

void NetClient::RecvCompletion_NET(){
	// 패킷 조립
	for (;;) {
		int recv_len = client_session.recv_buf.GetUseSize();
		if (recv_len < NET_HEADER_SIZE)
			break;

		// 헤더 카피
		PacketBuffer* encrypt_packet = PacketBuffer::Alloc();
		char* p_packet = encrypt_packet->Get_PacketPos_NET();
		client_session.recv_buf.Peek(p_packet, NET_HEADER_SIZE);

		BYTE code = ((NET_HEADER*)p_packet)->code;
		WORD payload_len = ((NET_HEADER*)p_packet)->len;

		// code 검사
		if (code != protocol_code) {
			PacketBuffer::Free(encrypt_packet);
			LOG("NetClient", LOG_LEVEL_WARN, "Recv Packet is wrong code!!", WSAGetLastError());
			DisconnectSession();
			break;
		}

		// 페이로드 데이터 부족
		if (recv_len < (NET_HEADER_SIZE + payload_len)) {
			PacketBuffer::Free(encrypt_packet);
			break;
		}

		// Recv Data 패킷 화
		client_session.recv_buf.Move_Front(NET_HEADER_SIZE);
		client_session.recv_buf.Dequeue(encrypt_packet->Get_writePos(), payload_len);
		encrypt_packet->Move_Wp(payload_len);

		// 복호패킷 생성
		PacketBuffer* decrypt_packet = PacketBuffer::Alloc();
		if (!decrypt_packet->DecryptPacket(encrypt_packet, private_key)) {
			PacketBuffer::Free(encrypt_packet);
			PacketBuffer::Free(decrypt_packet);
			LOG("NetClient", LOG_LEVEL_WARN, "Recv Packet is wrong checksum!!", WSAGetLastError());
			DisconnectSession();
			break;
		}

		// 사용자 패킷 처리
		OnRecv(decrypt_packet);
		InterlockedIncrement(&recvMsgTPS);

		// 암호패킷, 복호화 패킷 Free
		PacketBuffer::Free(encrypt_packet);
		PacketBuffer::Free(decrypt_packet);
	}

	//------------------------------
	// Post Recv
	//------------------------------
	if (!client_session.disconnect_flag) {
		AsyncRecv();
	}
}

bool NetClient::Check_InvalidSession() {
	IncrementIOCount();

	// 세션 릴리즈 상태
	if (true == client_session.release_flag) {
		DecrementIOCount();
		return false;
	}
	return true;
}

bool NetClient::Disconnect() {
	if (false == Check_InvalidSession()) return true;
	DisconnectSession();
	PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)&client_session, (LPOVERLAPPED)PQCS_TYPE::DECREMENT_IO);
	return true;
}

void NetClient::CleanUp() {
	// AcceptThread 종료
	if (acceptThread.joinable()) {
		acceptThread.join();
	}

	// WorkerThread 종료
	if (workerThread.joinable()) {
		workerThread.join();
	}

	// 세션 정리
	// ...

	closesocket(client_sock);
	CloseHandle(h_iocp);
	WSACleanup();
}

//------------------------------
// SESSION_ID
//------------------------------

SESSION_ID NetClient::Get_SessionID() {
	static DWORD unique = 1;
	DWORD index = 0;
	SESSION_ID session_id(index, unique++);
	return session_id;
}