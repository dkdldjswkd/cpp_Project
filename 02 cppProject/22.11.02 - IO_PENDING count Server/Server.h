#pragma once

extern SOCKET listen_sock;
extern SOCKET client_sock;

extern HANDLE write_event;
extern HANDLE recv_event;

struct Server {
	// º¯¼ö
	static bool shutdown;

	// Func
	static void StartUp();
	static void CleanUp();
	static void AcceptProc();
	static void RecvProc();
	static void CALLBACK Recv_APC(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
};
