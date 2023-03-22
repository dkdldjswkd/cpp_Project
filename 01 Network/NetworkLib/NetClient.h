#pragma once
#include <Windows.h>
#include <thread>
#include "Session.h"
#include "PacketBuffer.h"
#include "../../00 lib_jy/RingBuffer.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../../00 lib_jy/LFStack.h"
#include "../../00 lib_jy/Parser.h"


//------------------------------
// NetworkLib
//------------------------------
class NetClient {
public:
	NetClient(const char* systemFile, const char* client);
	virtual ~NetClient();

private:
	friend PacketBuffer;

private:
	enum class NetType : BYTE {
		LAN,
		NET,
		NONE,
	};

	enum class PQCS_TYPE : BYTE {
		SEND_POST = 1,
		RELEASE_SESSION = 2,
		NONE,
	};

private:
	// 프로토콜
	BYTE protocolCode;
	BYTE privateKey;

	// 네트워크
	NetType netType;
	SOCKET clientSock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	char serverIP[16] = { 0, };
	WORD serverPort = 0;

	// 세션
	Session clientSession;

	// 스레드
	std::thread workerThread;
	std::thread connectThread;

	// 옵션
	bool reconnectFlag = true;

	// 모니터링
	DWORD recvMsgTPS = 0;
	DWORD sendMsgTPS = 0;
	alignas(64) DWORD recvMsgCount = 0;
	alignas(64) DWORD sendMsgCount = 0;

private:
	// 스레드
	void WorkerFunc();
	void ConnectFunc();

	// IO 완료 통지 루틴
	void RecvCompletionLAN();
	void RecvCompletionNET();
	void SendCompletion();

	// 세션
	SESSION_ID GetSessionID();
	bool ValidateSession();
	inline void IncrementIOCount();
	inline void DecrementIOCount();
	inline void DecrementIOCountPQCS();
	inline void DisconnectSession();

	// Send/Recv
	bool SendPost();
	int	 AsyncSend();
	bool AsyncRecv();

	// 컨텐츠
	void ReleaseSession();

protected:
	// 라이브러리 사용자 측 재정의 하여 사용
	virtual void OnConnect() = 0;
	virtual void OnRecv(PacketBuffer* contents_packet) = 0;
	virtual void OnDisconnect() = 0;
	virtual void OnClientStop() = 0;
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;       
	// virtual void OnWorkerThreadBegin() = 0;                 
	// virtual void OnWorkerThreadEnd() = 0;                   

public:
	// 유틸
	Parser parser;

public:
	void Start();
	void Stop();

public:
	void SendPacket(PacketBuffer* send_packet);
	bool Disconnect();

public:
	// 모니터링 Getter
	void UpdateTPS();
	DWORD GetSendTPS();
	DWORD GetRecvTPS();
};

inline void NetClient::DecrementIOCount() {
	if (0 == InterlockedDecrement((LONG*)&clientSession.ioCount)) {
		ReleaseSession();
	}
}

inline void NetClient::DecrementIOCountPQCS() {
	if (0 == InterlockedDecrement((LONG*)&clientSession.ioCount)) {
		PostQueuedCompletionStatus(h_iocp, 1, (ULONG_PTR)&clientSession, (LPOVERLAPPED)PQCS_TYPE::RELEASE_SESSION);
	}
}

inline void NetClient::IncrementIOCount() {
	InterlockedIncrement((LONG*)&clientSession.ioCount);
}

// * 'IO Count == 0' 이 될 수 없을때 사용할것. (그렇지 않다면, 다른세션 끊는문제 발생)
inline void NetClient::DisconnectSession() {
	clientSession.disconnectFlag = true;
	CancelIoEx((HANDLE)clientSession.sock, NULL);
}

////////////////////////////// 
// Getter
////////////////////////////// 

inline void NetClient::UpdateTPS() {
	sendMsgTPS = sendMsgCount;
	sendMsgCount = 0;

	recvMsgTPS = recvMsgCount;
	recvMsgCount = 0;
}

inline DWORD NetClient::GetRecvTPS() {
	return recvMsgTPS;
}

inline DWORD NetClient::GetSendTPS() {
	return sendMsgTPS;
}
