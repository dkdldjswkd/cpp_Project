#pragma once

#define SERVER_IP INADDR_ANY
#define SERVER_PORT 3000

#define MAX_BUF 256

#define MAX_CLIENT 63

#include <list>
#include "Render.h"
struct Player;

class StarServer {
private:
	StarServer();
public:
	static StarServer& Get_Inst();

private:
	SOCKET listen_socket = INVALID_SOCKET;
	std::list<Player> players;
	char recv_buf[MAX_BUF] = { 0, };

public:

private:
	friend void Render();

private:
	bool RecvProc(Player* p_player); // RecvProc(플레이어 포인터)
	bool SendUnicast(void* p_packet, Player* p_player);
	bool SendBroadCast(void* p_packet, const char* exclude = nullptr); //SendBroadCast(메시지, 제외할 대상)
	bool AcceptProc();
	void Disconnect();
	int Get_id();

public:
	bool Init();
	void Run();
	void Close();
};
