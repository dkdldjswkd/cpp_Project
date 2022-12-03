#pragma once
#include "RingBuffer.h"
#include <queue>
#include <vector>
#include "Packet.h"

#define SERVER_IP INADDR_ANY
#define SERVER_PORT 5000

#define MAX_USER 63
#define NO_USER_INDEX (MAX_USER * 2)
#define INVALID_ID ~0

#define BUF_SIZE 256
#define MAX_PACKET_SIZE 64

#define FRAME_PER_SEC 50
#define FRAME_TIME (DWORD)(1000/FRAME_PER_SEC)

struct Session {
	Session();
	~Session();

public:
	void Set(SOCKET s, ULONG ip , USHORT port, unsigned short id);
	void Disconnect();

public:
	SOCKET sock = INVALID_SOCKET;
	ULONG ip = 0;
	USHORT port = 0;
	unsigned short id = INVALID_ID;

	RingBuffer send_buf;
	RingBuffer recv_buf;

	bool disconnect_flag = false;

public:
	unsigned char dir = PACKET_MOVE_DIR_LL;
	unsigned short x = 300;
	unsigned short y = 300;
	char hp = 100;
	bool is_move = false;
};

extern Session sessions[MAX_USER];
unsigned short Get_Session_No();

extern SOCKET listen_sock;
extern SOCKADDR_IN server_addr;
extern int client_addr_size;
//extern std::queue<User_Info*> disconnect_queue;

bool Server_Init();
bool Server_Run();
bool Server_Close();

void NetWork_Process();
void Accept_Proc(Session* user_info);
void Recv_Proc(Session* user_info);
void Close_Proc(Session* user_info);
void Send_Proc(Session* user_info);

void Update_Logic();

// 인라인 고려
int Unicast(Session* user_info, const char* packet, const unsigned short packet_size);
void Broadcast(std::vector<Session*>* exclude_users, const char* packet, const unsigned short packet_size);
void Unicast_packet_sc_create_my_character(
	Session* user_info,
	unsigned int id,
	unsigned char dir,
	unsigned short x,
	unsigned short y,
	char hp
);