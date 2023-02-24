#pragma once
#include <Windows.h>
#include <thread>
#include "Session.h"
#include "PacketBuffer.h"
#include "RingBuffer.h"
#include "LFQueue.h"
#include "LFStack.h"
#include "Parser.h"

enum class NetType : BYTE {
	LAN,
	NET,
	NONE,
};

//------------------------------
// NetworkLib
//------------------------------
class NetClient {
public:
	NetClient(const char* systemFile, const char* server);
	virtual ~NetClient();

private:
	friend PacketBuffer;

private:
	enum class PQCS_TYPE : BYTE {
		SEND_POST = 1,
		DECREMENT_IO = 2,
	};

private:
	// 프로토콜
	BYTE protocol_code;
	BYTE private_key;

	// 네트워크
	NetType netType;
	SOCKET client_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	char server_ip[16] = { 0, };
	WORD server_port = 0;

	// 세션
	Session client_session;

	// 스레드
	std::thread workerThread;
	std::thread acceptThread;

	// 옵션
	bool nagle_flag;

private:
	// 모니터링
	alignas(64) DWORD recvMsgTPS = 0;
	alignas(64) DWORD sendMsgTPS = 0;

private:
	// 스레드
	void WorkerFunc();

	// IO 완료 통지 루틴
	void RecvCompletion_LAN();
	void RecvCompletion_NET();
	void SendCompletion();

	// 세션
	SESSION_ID Get_SessionID();
	bool Check_InvalidSession();
	inline void DecrementIOCount();
	inline void IncrementIOCount();
	inline void DisconnectSession();

	// Send/Recv
	bool SendPost();
	int	 AsyncSend();
	bool AsyncRecv();

	// 컨텐츠
	bool ReleaseSession();

protected:
	// 라이브러리 사용자 측 재정의 하여 사용
	virtual void OnConnect() = 0;		// 접속 처리 후 호출
	virtual void OnRecv(PacketBuffer* contents_packet) = 0;
	virtual void OnDisconnect() = 0; // Release 후 호출
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;           // 패킷 송신 완료 후
	// virtual void OnWorkerThreadBegin() = 0;                    // 워커스레드 GQCS 바로 하단에서 호출
	// virtual void OnWorkerThreadEnd() = 0;                      // 워커스레드 1루프 종료 후

public:
	// 유틸
	Parser parser;

public:
	void StartUp();
	void CleanUp();

public:
	void SendPacket(PacketBuffer* send_packet);
	bool Disconnect();

public:
	// 모니터링 Getter
	DWORD Get_sendTPS();
	DWORD Get_recvTPS();
};

inline void NetClient::DecrementIOCount() {
	if (0 == InterlockedDecrement((LONG*)&client_session.io_count))
		ReleaseSession();
}

inline void NetClient::IncrementIOCount() {
	InterlockedIncrement((LONG*)&client_session.io_count);
}

// * 'IO Count == 0' 이 될 수 없을때 사용할것. (그렇지 않다면, 다른세션 끊는문제 발생)
inline void NetClient::DisconnectSession() {
	client_session.disconnect_flag = true;
	CancelIoEx((HANDLE)client_session.sock, NULL);
}

inline DWORD NetClient::Get_sendTPS() {
	auto tmp = sendMsgTPS;
	sendMsgTPS = 0;
	return tmp;
}

inline DWORD NetClient::Get_recvTPS() {
	auto tmp = recvMsgTPS;
	recvMsgTPS = 0;
	return tmp;
}