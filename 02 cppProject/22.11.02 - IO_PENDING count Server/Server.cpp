#include "stdafx.h"
#include "Server.h"
#include "Define.h"
#include "Session.h"
#include "../../J_LIB/ObjectPool/ObjectPool.h"
#pragma comment(lib, "../../J_LIB/ObjectPool/ObjectPool.lib")

SOCKET listen_sock;
SOCKET client_sock;
HANDLE read_event;
HANDLE write_event;
bool Server::shutdown = false;

using namespace std;

// 자료구조
unordered_map<SESSION_ID, Session*> session_map;

// 오브젝트 풀
ObjectPool<Session> session_pool;

// 스레드 큐
queue<SESSION_ID> recvJob_queue;
mutex recvQ_lock;
HANDLE recv_event = CreateEvent(NULL, FALSE, FALSE, NULL);

unsigned int recv_call = 0;
unsigned int recv_IO_PENDING = 0;
unsigned int send_call = 0;
unsigned int send_IO_PENDING = 0;

void Server::StartUp() {
	WSADATA wsa_data;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa_data))
		throw;

	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET)
		throw;

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);

	auto ret_bind = bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr));
	if (SOCKET_ERROR == ret_bind)
		throw;

	auto ret_listen = listen(listen_sock, SOMAXCONN);
	if (SOCKET_ERROR == ret_listen)
		throw;
}

void Server::CleanUp(){
	WSACleanup();
}

void Server::AcceptProc(){
	printf("Start AcceptProc \n");

	for (;;) {
		client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == INVALID_SOCKET)
			break;

		printf("Accept \n");

		// 세션 추가
		auto accept_id = Session::Get_SessionID();
		auto p_session = session_pool.Alloc();
		p_session->Clear();
		p_session->Set(client_sock, accept_id);

		session_map.insert({ accept_id, p_session });
		recvQ_lock.lock();
		recvJob_queue.push(accept_id);
		recvQ_lock.unlock();
		SetEvent(recv_event);
	}

	printf("End AcceptProc \n");
}

void Server::RecvProc(){
	printf("Start RecvProc \n");

	for (;;) {
		for (;;) {
			auto ret = WaitForSingleObjectEx(recv_event, INFINITE, TRUE);
			if (WAIT_IO_COMPLETION) break;
			if (WAIT_OBJECT_0) break;
		}

		while (!recvJob_queue.empty()) {
			recvQ_lock.lock();
			SESSION_ID id = recvJob_queue.front();
			recvJob_queue.pop();
			recvQ_lock.unlock();

			auto p_session = session_map.find(id)->second;
			ZeroMemory(&p_session->overlapped, sizeof(p_session->overlapped));
			DWORD flag = 0;

			recv_call++;
			auto ret = WSARecv(p_session->sock, &p_session->wsa_recv, 1, NULL, &flag, &p_session->overlapped, Server::Recv_APC);
			if (ret == SOCKET_ERROR) {
				auto err_no = GetLastError();
				switch (err_no)	{
					case ERROR_IO_PENDING: {
						recv_IO_PENDING++;
						break;
					}
					case  WSAECONNRESET: {
						recv_call--;
						break;
					}
				
					default: {
						printf("[session id : %d] SOCKET ERROR : %d \n", id, err_no);
					}
				}
			}
		}
	}

	printf("End RecvProc \n");
}

void Server::Recv_APC(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags){
	if (cbTransferred == 0)
		return;

	Session* p_session = (Session*)lpOverlapped;
	p_session->wsa_recv.buf[cbTransferred] = 0;

	printf("[session id : %d] recv(%d byte) : %s \n", p_session->id, cbTransferred, p_session->wsa_recv.buf);
	memmove(p_session->wsa_send.buf, p_session->wsa_recv.buf, cbTransferred);
	p_session->wsa_send.len = cbTransferred;

	send_call++;
	DWORD err_no;
	auto ret = WSASend(p_session->sock, &p_session->wsa_send, 1, NULL, 0, &p_session->overlapped, nullptr);
	if (ret == SOCKET_ERROR) {
		err_no = GetLastError();
		switch (err_no) {
		case ERROR_IO_PENDING: {
			send_IO_PENDING++;
			break;
		}
		case  WSAECONNRESET: {
			send_call--;
			break;
		}

		default:
			break;
		}
	}

	if (ret != SOCKET_ERROR || err_no == ERROR_IO_PENDING) {
		recvQ_lock.lock();
		recvJob_queue.push(p_session->id);
		recvQ_lock.unlock();
	}
}