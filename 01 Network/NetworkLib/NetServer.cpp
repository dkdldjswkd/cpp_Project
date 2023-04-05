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

using namespace std;

//------------------------------
// Server Func
//------------------------------
NetServer::NetServer(const char* systemFile, const char* server) {
	int serverPort;
	int nagle;

	// Read SystemFile
	parser.LoadFile(systemFile);
	parser.GetValue(server, "PROTOCOL_CODE", (int*)&protocolCode);
	parser.GetValue(server, "PRIVATE_KEY", (int*)&privateKey);
	parser.GetValue(server, "NET_TYPE", (int*)&netType);
	parser.GetValue(server, "PORT", (int*)&serverPort);
	parser.GetValue(server, "MAX_SESSION", (int*)&maxSession);
	parser.GetValue(server, "NAGLE", (int*)&nagle);
	parser.GetValue(server, "TIME_OUT_FLAG", (int*)&timeoutFlag);
	parser.GetValue(server, "TIME_OUT", (int*)&timeOut);
	parser.GetValue(server, "TIME_OUT_CYCLE", (int*)&timeoutCycle);
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

	listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listenSock) {
		LOG("NetServer", LOG_LEVEL_FATAL, "WSASocket()_ERROR : %d", WSAGetLastError());
		throw std::exception("WSASocket_ERROR");
	}

	// Reset nagle
	if (!nagle) {
		int opt_val = TRUE;
		if (setsockopt(listenSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val)) != 0) {
			LOG("NetServer", LOG_LEVEL_FATAL, "SET_NAGLE_ERROR : %d", WSAGetLastError());
			throw std::exception("SET_NAGLE_ERROR");
		}
	}

	// Set Linger
	LINGER linger = { 1, 0 };
	if (setsockopt(listenSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof linger) != 0) {
		LOG("NetServer", LOG_LEVEL_FATAL, "SET_LINGER_ERROR : %d", WSAGetLastError());
		throw std::exception("SET_LINGER_ERROR");
	}

	// bind 
	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverPort);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if (0 != bind(listenSock, (SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN))) {
		LOG("NetServer", LOG_LEVEL_FATAL, "bind()_ERROR : %d", WSAGetLastError());
		throw std::exception("bind()_ERROR");
	}

	// Set Session
	sessionArray = new Session[maxSession];
	for (int i = 0; i < maxSession; i++)
		sessionIdxStack.Push(maxSession - (1 + i));

	// Create IOCP
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, activeWorker);
	if (h_iocp == INVALID_HANDLE_VALUE || h_iocp == NULL) {
		LOG("NetServer", LOG_LEVEL_FATAL, "CreateIoCompletionPort()_ERROR : %d", GetLastError());
		throw std::exception("CreateIoCompletionPort()_ERROR");
	}

	// Create IOCP Worker
	workerThreadArr = new thread[maxWorker];
	for (int i = 0; i < maxWorker; i++) {
		workerThreadArr[i] = thread([this] {WorkerFunc(); });
	}
}

NetServer::~NetServer() {
	delete[] sessionArray;
	delete[] workerThreadArr;
}

void NetServer::Start() {
	// listen
	if (0 != listen(listenSock, SOMAXCONN_HINT(65535))) {
		LOG("NetServer", LOG_LEVEL_FATAL, "listen()_ERROR : %d", WSAGetLastError());
		throw std::exception("listen()_ERROR");
	}

	// Create Thread
	acceptThread = thread([this] {AcceptFunc(); });
	if (timeoutFlag) {
		timeOutThread = thread([this] {TimeoutFunc(); });
	}

	// LOG
	LOG("NetServer", LOG_LEVEL_INFO, "Start NetServer");
}

void NetServer::AcceptFunc() {
	for (;;) {
		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);

		//------------------------------
		// Accept
		//------------------------------
		auto acceptSock = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
		acceptTotal++;
		if ( acceptSock == INVALID_SOCKET ) {
			auto errNo = WSAGetLastError();
			// 서버 종료
			if (errNo == WSAEINTR || errNo == WSAENOTSOCK ) {
				acceptTotal--;
				break;
			}
			// accept Error
			else {
				LOG("NetServer", LOG_LEVEL_WARN, "accept() Fail, Error code : %d", errNo);
				continue;
			}
		}
		if (maxSession <= sessionCount) {
			closesocket(acceptSock);
		}

		//------------------------------
		// 연결 수락 판단
		//------------------------------
		in_addr acceptIp = clientAddr.sin_addr;
		WORD acceptPort = ntohs(clientAddr.sin_port);
		if (false == OnConnectionRequest(acceptIp, acceptPort)) {
			closesocket(acceptSock);
			continue;
		}

		//------------------------------
		// 세션 할당
		//------------------------------

		// ID 할당
		SessionId sessionId = GetSessionId();
		if (sessionId == INVALID_SESSION_ID) {
			closesocket(acceptSock);
			continue;
		}

		// 세션 자료구조 할당
		Session* p_acceptSession = &sessionArray[sessionId.SESSION_INDEX];
		p_acceptSession->Set(acceptSock, acceptIp, acceptPort, sessionId);
		acceptCount++;

		// Bind IOCP
		CreateIoCompletionPort((HANDLE)acceptSock, h_iocp, (ULONG_PTR)p_acceptSession, 0);

		//------------------------------
		// 세션 로그인 시 작업
		//------------------------------
		OnClientJoin(sessionId.sessionId);
		InterlockedIncrement((LONG*)&sessionCount);

		//------------------------------
		// WSARecv
		//------------------------------
		AsyncRecv(p_acceptSession);

		// 생성 I/O Count 차감
		DecrementIOCount(p_acceptSession);
	}
}

void NetServer::TimeoutFunc() {
	for (;;) {
		Sleep(timeoutCycle);
		INT64 cur_time = timeGetTime();
		for (int i = 0; i < maxSession; i++) {
			Session* p_session = &sessionArray[i];
			SessionId id = p_session->sessionId;

			// 타임아웃 처리 대상 아님 (Interlocked 연산 최소화하기 위해)
			if (sessionArray[i].releaseFlag || (timeOut > cur_time - sessionArray[i].lastRecvTime))
				continue;

			// 타임아웃 처리 대상일 수 있음
			IncrementIOCount(p_session);
			// 릴리즈 세션인지 판단.
			if (true == p_session->releaseFlag) {
				DecrementIOCount(p_session);
				continue;
			}
			// 타임아웃 조건 판단.
			if (timeOut > cur_time - sessionArray[i].lastRecvTime) {
				DecrementIOCount(p_session);
				continue;
			}

			// 타임아웃 처리
			DisconnectSession(p_session);
			DecrementIOCount(p_session);
		}
	}
}

void NetServer::WorkerFunc() {
	for (;;) {
		DWORD ioSize = 0;
		Session* p_session = 0;
		LPOVERLAPPED p_overlapped = 0;

		BOOL retGQCS = GetQueuedCompletionStatus(h_iocp, &ioSize, (PULONG_PTR)&p_session, &p_overlapped, INFINITE);

		// 워커 스레드 종료
		if (ioSize == 0 && p_session == 0 && p_overlapped == 0) {
			PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
			return;
		}
		// FIN
		if (ioSize == 0) {
			if (&p_session->sendOverlapped == p_overlapped)
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

		// recv 완료통지
		if (&p_session->recvOverlapped == p_overlapped) {
			if (retGQCS) {
				p_session->recvBuf.MoveRear(ioSize);
				p_session->lastRecvTime = timeGetTime();
				// 내부 통신 외부 통신 구분
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
		// send 완료통지
		else if (&p_session->sendOverlapped == p_overlapped) {
			if (retGQCS) {
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
	if (0 != p_session->ioCount)
		return false;

	// * releaseFlag(0), iocount(0) -> releaseFlag(1), iocount(0)
	if (0 == InterlockedCompareExchange64((long long*)&p_session->releaseFlag, 1, 0)) {
		// 리소스 정리 (소켓, 패킷)
		closesocket(p_session->sock);
		PacketBuffer* packet;
		while (p_session->sendQ.Dequeue(&packet)) {
			PacketBuffer::Free(packet);
		}
		for (int i = 0; i < p_session->sendPacketCount; i++) {
			PacketBuffer::Free(p_session->sendPacketArr[i]);
		}

		// 사용자 리소스 정리
		OnClientLeave(p_session->sessionId);

		// 세션 반환
		sessionIdxStack.Push(p_session->sessionId.SESSION_INDEX);
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
	InterlockedAdd((LONG*)&sendMsgCount, p_session->sendPacketCount);
	p_session->sendPacketCount = 0;

	// Send Flag OFF
	InterlockedExchange8((char*)&p_session->sendFlag, false);

	// Send 조건 체크
	if (p_session->disconnectFlag) return;
	if (p_session->sendQ.GetUseCount() <= 0) return;
	SendPost(p_session);
}

// SendQ Enqueue, SendPost
void NetServer::SendPacket(SessionId sessionId, PacketBuffer* sendPacket) {
	Session* p_session = ValidateSession(sessionId);
	if (nullptr == p_session)
		return;

	if (p_session->disconnectFlag) {
		DecrementIOCountPQCS(p_session);
		return;
	}

	// LAN, NET 구분
	if (NetType::LAN == netType) {
		sendPacket->SetLanHeader();
		sendPacket->IncrementRefCount();
	}
	else {
		sendPacket->SetNetHeader(protocolCode, privateKey);
		sendPacket->IncrementRefCount();
	}

	p_session->sendQ.Enqueue(sendPacket);
#if PQCS_SEND
	if (p_session->sendFlag == false) {
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

// AsyncSend Call 시도
bool NetServer::SendPost(Session* p_session) {
	// Empty return
	if (p_session->sendQ.GetUseCount() <= 0)
		return false;

	// Send 1회 체크 (send flag, true 시 send 진행 중)
	if (p_session->sendFlag == true)
		return false;
	if (InterlockedExchange8((char*)&p_session->sendFlag, true) == true)
		return false;

	// Empty continue
	if (p_session->sendQ.GetUseCount() <= 0) {
		InterlockedExchange8((char*)&p_session->sendFlag, false);
		return SendPost(p_session);
	}

	AsyncSend(p_session);
	return true;
}

// WSASend() call
int NetServer::AsyncSend(Session* p_session) {
	WSABUF wsaBuf[MAX_SEND_MSG];

	// MAX SEND 제한 초과
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
	ZeroMemory(&p_session->sendOverlapped, sizeof(p_session->sendOverlapped));
	if (SOCKET_ERROR == WSASend(p_session->sock, wsaBuf, p_session->sendPacketCount , NULL, 0, &p_session->sendOverlapped, NULL)) {
		const auto err_no = WSAGetLastError();
		if (ERROR_IO_PENDING != err_no) { // Send 실패
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
	wsaBuf[0].buf = p_session->recvBuf.GetWritePos();
	wsaBuf[0].len = p_session->recvBuf.DirectEnqueueSize();
	// Recv Remain Pos
	wsaBuf[1].buf = p_session->recvBuf.GetBeginPos();
	wsaBuf[1].len = p_session->recvBuf.RemainEnqueueSize();

	IncrementIOCount(p_session);
	ZeroMemory(&p_session->recvOverlapped, sizeof(p_session->recvOverlapped));
	if (SOCKET_ERROR == WSARecv(p_session->sock, wsaBuf, 2, NULL, &flags, &p_session->recvOverlapped, NULL)) {
		if (WSAGetLastError() != ERROR_IO_PENDING) { // Recv 실패
			LOG("NetServer", LOG_LEVEL_DEBUG, "WSARecv() Fail, Error code : %d", WSAGetLastError());
			DecrementIOCount(p_session);
			return false;
		}
	}

	// Disconnect 체크
	if (p_session->disconnectFlag) {
		CancelIoEx((HANDLE)p_session->sock, NULL);
		return false;
	}
	return true;
}

void NetServer::RecvCompletionLan(Session* p_session) {
	// 패킷 조립
	for (;;) {
		int recvLen = p_session->recvBuf.GetUseSize();
		if (recvLen <= LAN_HEADER_SIZE)
			break;

		LanHeader lanHeader;
		p_session->recvBuf.Peek(&lanHeader, LAN_HEADER_SIZE);

		// 페이로드 데이터 부족
		if (recvLen < lanHeader.len + LAN_HEADER_SIZE)
			break;

		//------------------------------
		// OnRecv (네트워크 헤더 제거)
		//------------------------------
		PacketBuffer* contentsPacket = PacketBuffer::Alloc();

		// 컨텐츠 패킷 생성
		p_session->recvBuf.MoveFront(LAN_HEADER_SIZE);
		p_session->recvBuf.Dequeue(contentsPacket->writePos, lanHeader.len);
		contentsPacket->MoveWp(lanHeader.len);

		// 사용자 패킷 처리
		OnRecv(p_session->sessionId, contentsPacket);
		InterlockedIncrement(&recvMsgCount);

		PacketBuffer::Free(contentsPacket);
	}

	//------------------------------
	// Post Recv (Recv 걸어두기)
	//------------------------------
	if (false == p_session->disconnectFlag) {
		AsyncRecv(p_session);
	}
}

void NetServer::RecvCompletionNet(Session* p_session) {
	// 패킷 조립
	for (;;) {
		int recvLen = p_session->recvBuf.GetUseSize();
		if (recvLen <= NET_HEADER_SIZE)
			break;

		// 헤더 카피
		char encryptPacket[NET_HEADER_SIZE + MAX_PAYLOAD_LEN];
		p_session->recvBuf.Peek(encryptPacket, NET_HEADER_SIZE);

		// code 검사
		BYTE code = ((NetHeader*)encryptPacket)->code;
		if (code != protocolCode) {
			LOG("NetServer", LOG_LEVEL_WARN, "Recv Packet is wrong code!!", WSAGetLastError());
			DisconnectSession(p_session);
			break;
		}

		// 페이로드 데이터 부족
		WORD payloadLen = ((NetHeader*)encryptPacket)->len;
		if (recvLen < (NET_HEADER_SIZE + payloadLen)) {
			break;
		}

		// 암호패킷 복사
		p_session->recvBuf.MoveFront(NET_HEADER_SIZE);
		p_session->recvBuf.Dequeue(encryptPacket + NET_HEADER_SIZE, payloadLen);

		// 패킷 복호화
		PacketBuffer* decrypt_packet = PacketBuffer::Alloc();
		if (!decrypt_packet->DecryptPacket(encryptPacket, privateKey)) {
			PacketBuffer::Free(decrypt_packet);
			LOG("NetServer", LOG_LEVEL_WARN, "Recv Packet is wrong checksum!!", WSAGetLastError());
			DisconnectSession(p_session);
			break;
		}

		// 사용자 패킷 처리
		OnRecv(p_session->sessionId, decrypt_packet);
		InterlockedIncrement(&recvMsgCount);

		// 암호패킷, 복호화 패킷 Free
		PacketBuffer::Free(decrypt_packet);
	}

	//------------------------------
	// Post Recv
	//------------------------------
	if (!p_session->disconnectFlag) {
		AsyncRecv(p_session);
	}
}

Session* NetServer::ValidateSession(SessionId sessionId) {
	Session* p_session = &sessionArray[sessionId.SESSION_INDEX];
	IncrementIOCount(p_session);

	// 세션 릴리즈 상태
	if (true == p_session->releaseFlag) {
		DecrementIOCountPQCS(p_session);
		return nullptr;
	}
	// 세션 재사용
	if (p_session->sessionId != sessionId) {
		DecrementIOCountPQCS(p_session);
		return nullptr;
	}

	// 재사용 가능성 제로
	return p_session;
}

void NetServer::Disconnect(SessionId session_id) {
	Session* p_session = ValidateSession(session_id);
	if (nullptr == p_session) return;
	DisconnectSession(p_session);
	DecrementIOCountPQCS(p_session);
}

void NetServer::Stop() {
	// AcceptThread 종료
	closesocket(listenSock);
	if (acceptThread.joinable()) {
		acceptThread.join();
	}

	// 세션 정리
	for (int i = 0; i < maxSession; i++) {
		Session* p_session = &sessionArray[i];
		if (true == p_session->releaseFlag)
			continue;
		DisconnectSession(p_session);
	}

	// 세션 정리 체크
	for (;;) {
		if (sessionCount == 0) {
			break;
		}
		Sleep(100);
	}

	// Worker 종료
	PostQueuedCompletionStatus(h_iocp, 0, 0, 0);
	for (int i = 0; i < maxWorker; i++) {
		if (workerThreadArr[i].joinable()) {
			workerThreadArr[i].join();
		}
	}

	CloseHandle(h_iocp);
	WSACleanup();

	// 사용자 리소스 정리
	OnServerStop();
}

//------------------------------
// SESSION_ID
//------------------------------

SessionId NetServer::GetSessionId() {
	DWORD index;
	if (false == sessionIdxStack.Pop(&index)) {
		return INVALID_SESSION_ID;
	}

	SessionId sessionId(index, ++sessionUnique);
	return sessionId;
}