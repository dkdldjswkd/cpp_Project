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
	parser.GetValue(server, "PROTOCOL_CODE", (int*)&protocolCode);
	parser.GetValue(server, "PRIVATE_KEY", (int*)&privateKey);
	parser.GetValue(server, "NET_TYPE", (int*)&netType);
	parser.GetValue(server, "IP", serverIP);
	parser.GetValue(server, "PORT", (int*)&serverPort);

	// Check system
	if (1 < (BYTE)netType) CRASH();

	//////////////////////////////
	// Set Client
	//////////////////////////////

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		LOG("NetClient", LOG_LEVEL_FATAL, "WSAStartup() Eror(%d)", WSAGetLastError());
		CRASH();
	}

	// Create IOCP
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 2);
	if (INVALID_HANDLE_VALUE == h_iocp) CRASH();

	// Create IOCP Worker Thread
	workerThread = thread([this] { WorkerFunc(); });
}
NetClient::~NetClient() {}

void NetClient::Start() {
	connectThread = thread([this]() {ConnectFunc(); });
	LOG("NetClient", LOG_LEVEL_DEBUG, "Client Start");
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
			if (&p_session->sendOverlapped == p_overlapped)
				LOG("NetClient", LOG_LEVEL_FATAL, "Zero Byte Send !!");
			goto Decrement_IOCount;
		}
		// PQCS
		else if ((ULONG_PTR)p_overlapped < (ULONG_PTR)PQCS_TYPE::NONE) {
			switch ((PQCS_TYPE)(byte)p_overlapped) {
			case PQCS_TYPE::SEND_POST: {
				SendPost();
				goto Decrement_IOCount;
			}

			case PQCS_TYPE::RELEASE_SESSION: {
				ReleaseSession();
				continue;
			}

			default:
				LOG("NetServer", LOG_LEVEL_FATAL, "PQCS Default");
				break;
			}
		}

		// recv 완료통지
		if (&p_session->recvOverlapped == p_overlapped) {
			if (ret_GQCS) {
				p_session->recv_buf.MoveRear(io_size);
				p_session->lastRecvTime = timeGetTime();
				// 내부 통신 외부 통신 구분
				if (NetType::LAN == netType) {
					RecvCompletionLAN();
				}
				else {
					RecvCompletionNET();
				}
			}  
			else {
				LOG("NetClient", LOG_LEVEL_DEBUG, "Overlapped Recv Fail");
			}
		}
		// send 완료통지
		else if (&p_session->sendOverlapped == p_overlapped) {
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

void NetClient::ConnectFunc() {
	// Create Socket
	clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == clientSock) {
		LOG("NetClient", LOG_LEVEL_FATAL, "WSASocket() Eror(%d)", WSAGetLastError());
		CRASH();
	}

	// BIND IOCP
	CreateIoCompletionPort((HANDLE)clientSock, h_iocp, (ULONG_PTR)&clientSession, 0);

	// Set server address
	sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(serverPort);
	if (inet_pton(AF_INET, serverIP, &server_address.sin_addr) != 1) {
		LOG("NetClient", LOG_LEVEL_WARN, "inet_pton() Error(%d)", WSAGetLastError());
		return;
	}

	// Try Connect
	while (connect(clientSock, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
		LOG("NetClient", LOG_LEVEL_WARN, "connect() Error(%d)", WSAGetLastError());
		if (!reconnectFlag) {
			closesocket(clientSock);
			return;
		}
		Sleep(1000);
	}

	// Set client
	clientSession.Set(clientSock, server_address.sin_addr, serverPort, 0);

	// Connect Success
	OnConnect();
	AsyncRecv();

	// 생성 I/O Count 차감 (* Release() 에서 Connect 재연결 시 데드락)
	DecrementIOCountPQCS();
}

void NetClient::ReleaseSession(){
	if (0 != clientSession.ioCount)
		return;

	// * release_flag(0), iocount(0) -> release_flag(1), iocount(0)
	if (0 == InterlockedCompareExchange64((long long*)&clientSession.releaseFlag, 1, 0)) {
		// 리소스 정리 (소켓, 패킷)
		closesocket(clientSession.sock);
		PacketBuffer* packet;
		while (clientSession.sendQ.Dequeue(&packet)) {
			PacketBuffer::Free(packet);
		}
		for (int i = 0; i < clientSession.sendPacketCount; i++) {
			PacketBuffer::Free(clientSession.sendPacketArr[i]);
		}

		// 사용자 리소스 정리
		OnDisconnect();

		// Conncet 재시도
		if (reconnectFlag) {
			if (connectThread.joinable()) {
				connectThread.join();
			}
			connectThread = thread([this]() {ConnectFunc(); });
		}
	}
}

void NetClient::SendCompletion(){
	// Send Packet Free
	for (int i = 0; i < clientSession.sendPacketCount; i++) {
		PacketBuffer::Free(clientSession.sendPacketArr[i]);
		InterlockedIncrement(&sendMsgCount);
	}
	clientSession.sendPacketCount = 0;

	// Send Flag OFF
	InterlockedExchange8((char*)&clientSession.sendFlag, false);

	// Send 조건 체크
	if (clientSession.disconnectFlag)	return;
	if (clientSession.sendQ.GetUseCount() <= 0) return;
	SendPost();
}

// SendQ Enqueue, SendPost
void NetClient::SendPacket(PacketBuffer* send_packet) {
	if (false == ValidateSession())
		return;

	if (clientSession.disconnectFlag) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)&clientSession, (LPOVERLAPPED)PQCS_TYPE::RELEASE_SESSION);
		return;
	}

	// LAN, NET 구분
	if (NetType::LAN == netType) {
		send_packet->SetLanHeader();
		send_packet->IncrementRefCount();
	}
	else {
		send_packet->SetNetHeader(protocolCode, privateKey);
		send_packet->IncrementRefCount();
	}

	clientSession.sendQ.Enqueue(send_packet);
	PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)&clientSession, (LPOVERLAPPED)PQCS_TYPE::SEND_POST);
}

// AsyncSend Call 시도
bool NetClient::SendPost() {
	// Empty return
	if (clientSession.sendQ.GetUseCount() <= 0)
		return false;

	// Send 1회 체크 (send flag, true 시 send 진행 중)
	if (clientSession.sendFlag == true)
		return false;
	if (InterlockedExchange8((char*)&clientSession.sendFlag, true) == true)
		return false;

	// Empty continue
	if (clientSession.sendQ.GetUseCount() <= 0) {
		InterlockedExchange8((char*)&clientSession.sendFlag, false);
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
			if (clientSession.sendQ.GetUseCount() <= 0) {
				clientSession.sendPacketCount = i;
				break;
			}
			clientSession.sendQ.Dequeue((PacketBuffer**)&clientSession.sendPacketArr[i]);
			wsaBuf[i].buf = clientSession.sendPacketArr[i]->GetLanPacketPos();
			wsaBuf[i].len = clientSession.sendPacketArr[i]->GetLanPacketSize();
		}
	}
	else {
		for (int i = 0; i < MAX_SEND_MSG; i++) {
			if (clientSession.sendQ.GetUseCount() <= 0) {
				clientSession.sendPacketCount = i;
				break;
			}
			clientSession.sendQ.Dequeue((PacketBuffer**)&clientSession.sendPacketArr[i]);
			wsaBuf[i].buf = clientSession.sendPacketArr[i]->GetNetPacketPos();
			wsaBuf[i].len = clientSession.sendPacketArr[i]->GetNetPacketSize();
		}
	}
	// MAX SEND 제한 초과
	if (clientSession.sendPacketCount == 0) {
		clientSession.sendPacketCount = MAX_SEND_MSG;
		DisconnectSession();
		return false;
	}

	IncrementIOCount();
	ZeroMemory(&clientSession.sendOverlapped, sizeof(clientSession.sendOverlapped));
	if (SOCKET_ERROR == WSASend(clientSession.sock, wsaBuf, clientSession.sendPacketCount, NULL, 0, &clientSession.sendOverlapped, NULL)) {
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
	wsaBuf[0].buf = clientSession.recv_buf.Get_WritePos();
	wsaBuf[0].len = clientSession.recv_buf.Direct_EnqueueSize();
	// Recv Remain Pos
	wsaBuf[1].buf = clientSession.recv_buf.Get_BeginPos();
	wsaBuf[1].len = clientSession.recv_buf.Remain_EnqueueSize();

	IncrementIOCount();
	ZeroMemory(&clientSession.recvOverlapped, sizeof(clientSession.recvOverlapped));
	if (SOCKET_ERROR == WSARecv(clientSession.sock, wsaBuf, 2, NULL, &flags, &clientSession.recvOverlapped, NULL)) {
		if (WSAGetLastError() != ERROR_IO_PENDING) { // Recv 실패
			LOG("NetClient", LOG_LEVEL_DEBUG, "WSARecv() Fail, Error code : %d", WSAGetLastError());
			DecrementIOCount();
			return false;
		}
	}

	// Disconnect 체크
	if (clientSession.disconnectFlag) {
		CancelIoEx((HANDLE)clientSession.sock, NULL);
		return false;
	}
	return true;
}

void NetClient::RecvCompletionLAN(){
	// 패킷 조립
	for (;;) {
		int recv_len = clientSession.recv_buf.GetUseSize();
		if (recv_len <= LAN_HEADER_SIZE)
			break;

		LanHeader lanHeader;
		clientSession.recv_buf.Peek(&lanHeader, LAN_HEADER_SIZE);

		// 페이로드 데이터 부족
		if (recv_len < lanHeader.len + LAN_HEADER_SIZE)
			break;

		//------------------------------
		// OnRecv (네트워크 헤더 제거)
		//------------------------------
		PacketBuffer* contents_packet = PacketBuffer::Alloc();

		// 컨텐츠 패킷 생성
		clientSession.recv_buf.Move_Front(LAN_HEADER_SIZE);
		clientSession.recv_buf.Dequeue(contents_packet->writePos, lanHeader.len);
		contents_packet->MoveWp(lanHeader.len);

		// 사용자 패킷 처리
		OnRecv(contents_packet);
		InterlockedIncrement(&recvMsgCount);

		PacketBuffer::Free(contents_packet);
	}

	//------------------------------
	// Post Recv (Recv 걸어두기)
	//------------------------------
	if (false == clientSession.disconnectFlag) {
		AsyncRecv();
	}
}

void NetClient::RecvCompletionNET(){
	// 패킷 조립
	for (;;) {
		int recv_len = clientSession.recv_buf.GetUseSize();
		if (recv_len < NET_HEADER_SIZE)
			break;

		// 헤더 카피
		char encryptPacket[200];
		clientSession.recv_buf.Peek(encryptPacket, NET_HEADER_SIZE);

		// code 검사
		BYTE code = ((NetHeader*)encryptPacket)->code;
		if (code != protocolCode) {
			LOG("NetServer", LOG_LEVEL_WARN, "Recv Packet is wrong code!!", WSAGetLastError());
			DisconnectSession();
			break;
		}

		// 페이로드 데이터 부족
		WORD payload_len = ((NetHeader*)encryptPacket)->len;
		if (recv_len < (NET_HEADER_SIZE + payload_len)) {
			break;
		}

		// Recv Data 패킷 화
		clientSession.recv_buf.Move_Front(NET_HEADER_SIZE);
		clientSession.recv_buf.Dequeue(encryptPacket + NET_HEADER_SIZE, payload_len);

		// 복호패킷 생성
		PacketBuffer* decrypt_packet = PacketBuffer::Alloc();
		if (!decrypt_packet->DecryptPacket(encryptPacket, privateKey)) {
			PacketBuffer::Free(decrypt_packet);
			LOG("NetServer", LOG_LEVEL_WARN, "Recv Packet is wrong checksum!!", WSAGetLastError());
			DisconnectSession();
			break;
		}

		// 사용자 패킷 처리
		OnRecv(decrypt_packet);
		InterlockedIncrement(&recvMsgCount);

		// 암호패킷, 복호화 패킷 Free
		PacketBuffer::Free(decrypt_packet);
	}

	//------------------------------
	// Post Recv
	//------------------------------
	if (!clientSession.disconnectFlag) {
		AsyncRecv();
	}
}

bool NetClient::ValidateSession() {
	IncrementIOCount();

	// 세션 릴리즈 상태
	if (true == clientSession.releaseFlag) {
		DecrementIOCount();
		return false;
	}
	return true;
}

bool NetClient::Disconnect() {
	if (false == ValidateSession()) return true;
	DisconnectSession();
	DecrementIOCountPQCS();
	return true;
}

void NetClient::Stop() {
	// Connect Thread 종료
	reconnectFlag = false;
	if (connectThread.joinable()) {
		connectThread.join();
	}

	// 세션 정리
	if (!clientSession.releaseFlag) {
		DisconnectSession();
	}

	// 세션 정리 체크
	while (!clientSession.releaseFlag) {
		Sleep(100);
	}

	// Worker 종료
	PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
	if (workerThread.joinable()) {
		workerThread.join();
	}

	CloseHandle(h_iocp);
	WSACleanup();

	// 사용자 리소스 정리
	OnClientStop();
}

//------------------------------
// SESSION_ID
//------------------------------

SESSION_ID NetClient::GetSessionID() {
	static DWORD unique = 1;
	DWORD index = 0;
	SESSION_ID session_id(index, unique++);
	return session_id;
}