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
	// ��������
	BYTE protocol_code;
	BYTE private_key;

	// ��Ʈ��ũ
	NetType netType;
	SOCKET client_sock = INVALID_SOCKET;
	HANDLE h_iocp = INVALID_HANDLE_VALUE;
	char server_ip[16] = { 0, };
	WORD server_port = 0;

	// ����
	Session client_session;

	// ������
	std::thread workerThread;
	std::thread acceptThread;

	// �ɼ�
	bool nagle_flag;

private:
	// ����͸�
	alignas(64) DWORD recvMsgTPS = 0;
	alignas(64) DWORD sendMsgTPS = 0;

private:
	// ������
	void WorkerFunc();

	// IO �Ϸ� ���� ��ƾ
	void RecvCompletion_LAN();
	void RecvCompletion_NET();
	void SendCompletion();

	// ����
	SESSION_ID Get_SessionID();
	bool Check_InvalidSession();
	inline void DecrementIOCount();
	inline void IncrementIOCount();
	inline void DisconnectSession();

	// Send/Recv
	bool SendPost();
	int	 AsyncSend();
	bool AsyncRecv();

	// ������
	bool ReleaseSession();

protected:
	// ���̺귯�� ����� �� ������ �Ͽ� ���
	virtual void OnConnect() = 0;		// ���� ó�� �� ȣ��
	virtual void OnRecv(PacketBuffer* contents_packet) = 0;
	virtual void OnDisconnect() = 0; // Release �� ȣ��
	// virtual void OnError(int errorcode /* (wchar*) */) = 0;
	// virtual void OnSend(SessionID, int sendsize) = 0;           // ��Ŷ �۽� �Ϸ� ��
	// virtual void OnWorkerThreadBegin() = 0;                    // ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	// virtual void OnWorkerThreadEnd() = 0;                      // ��Ŀ������ 1���� ���� ��

public:
	// ��ƿ
	Parser parser;

public:
	void StartUp();
	void CleanUp();

public:
	void SendPacket(PacketBuffer* send_packet);
	bool Disconnect();

public:
	// ����͸� Getter
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

// * 'IO Count == 0' �� �� �� ������ ����Ұ�. (�׷��� �ʴٸ�, �ٸ����� ���¹��� �߻�)
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