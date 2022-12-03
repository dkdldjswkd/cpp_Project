#include "stdafx.h"
#include "Server.h"
#include "Packet.h"

using namespace std;

SOCKET listen_sock;
SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;
int client_addr_size = sizeof(client_addr);
Session sessions[MAX_USER];
queue<Session*> disconnect_queue;
Session* move_users[MAX_USER];

Session::Session() : send_buf(BUF_SIZE), recv_buf(BUF_SIZE) {}

Session::~Session() {
	if (sock != INVALID_SOCKET)
		closesocket(sock);
}

void Session::Set(SOCKET s, ULONG ip, USHORT port, unsigned short id) {
	sock = s;
	this->ip = ip;
	this->port = port;
	this->id = id;
	this->disconnect_flag = false;

	this->dir = PACKET_MOVE_DIR_LL;
	this->x = 300;
	this->y = 300;
	this->hp = 100;
	this->is_move = false;
}

void Session::Disconnect() {
	printf("[SOCKET : %d, id : %d] CLOSE \n", (int)this->sock, this->id);

	closesocket(this->sock);
	this->sock = INVALID_SOCKET;

	this->send_buf.ClearBuffer();
	this->recv_buf.ClearBuffer();
}

unsigned short Get_Session_No() {
	static unsigned short id = 0;
	return id++;
}

bool emplace_disconnect_queue(Session* user_info) {
	if (user_info->disconnect_flag)
		return false;
	if (user_info->sock == INVALID_SOCKET)
		return false;

	user_info->disconnect_flag = true;
	disconnect_queue.emplace(user_info);
	return true;
}


bool Push_MoveUsers(Session* user_info) {
	if (user_info->is_move)
		return true;

	for (auto& p_move_user : move_users) {
		if (p_move_user == nullptr) {
			p_move_user = user_info;
			user_info->is_move = true;
			return true;
		}
	}
	return false;
}

bool Pop_MoveUsers(Session* user_info) {
	if (user_info->is_move == false)
		return false;

	for (auto& p_move_user : move_users) {
		if (p_move_user == user_info) {
			p_move_user = nullptr;
			user_info->is_move = false;
			return true;
		}
	}
	return false;
}

bool Server_Init() {
	printf("Server_Init \n");

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		auto erro_num = WSAGetLastError();
		CRASH();
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		auto erro_num = WSAGetLastError();
		CRASH();
	}

	// addr setting
	server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	// bind()
	if (SOCKET_ERROR == bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr))) {
		auto erro_num = WSAGetLastError();
		CRASH();
	}

	LINGER opt = { 1, 0 };
	setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt));


	// listen ()
	if (SOCKET_ERROR == listen(listen_sock, SOMAXCONN))
		throw;

	return true;
}

void NetWork_Process() {
	fd_set read_set;
	fd_set write_set;

	// INIT SET
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	// SET SET
	FD_SET(listen_sock, &read_set);
	for (int i = 0; i < MAX_USER; i++) {
		if (sessions[i].sock != INVALID_SOCKET) {
			FD_SET(sessions[i].sock, &read_set);

			if (!sessions[i].send_buf.Empty()) {
				FD_SET(sessions[i].sock, &write_set);
			}
		}
	}

	// SELECT
	timeval t{ 0,0 };
	auto ret_select = select(NULL, &read_set, &write_set, NULL, &t);
	if (SOCKET_ERROR == ret_select) {
		auto erro_num = WSAGetLastError();
		CRASH();
	}

	// Check Accept
	if (FD_ISSET(listen_sock, &read_set) != 0) {
		for (Session& user_info : sessions) {
			if (user_info.sock == INVALID_SOCKET) {
				Accept_Proc(&user_info);
				break;
			}
		}
	}
	// Check Recv
	for (auto& user_info : sessions) {
		if (user_info.sock == INVALID_SOCKET)
			continue;

		if (FD_ISSET(user_info.sock, &read_set) != 0) {
			char tmp_buf[BUF_SIZE];
			auto recv_size = recv(user_info.sock, tmp_buf, BUF_SIZE, NULL);

			if (recv_size == 0) {
				Close_Proc(&user_info);
			}
			else if (recv_size == SOCKET_ERROR) {
				auto erro_num = WSAGetLastError();
				switch (erro_num) {
				case WSAECONNRESET:
					emplace_disconnect_queue(&user_info);
					break;
				default:
					CRASH();
					break;
				}
			}
			else {
				auto ret_enqueue = user_info.recv_buf.Enqueue(tmp_buf, recv_size);
				if (ret_enqueue != recv_size) {
					if (user_info.recv_buf.Full()) {
						emplace_disconnect_queue(&user_info);
					}
					else {
						CRASH();
					}
				}
				Recv_Proc(&user_info);
			}
		}
	}
	// Check Send
	for (Session& user_info : sessions) {
		if (user_info.sock == INVALID_SOCKET)
			continue;

		if (FD_ISSET(user_info.sock, &write_set) != 0) {
			Send_Proc(&user_info);
		}
	}

	// Disconnect
	for (int i = 0; i < disconnect_queue.size(); i++) {
		disconnect_queue.front()->Disconnect();
		disconnect_queue.pop();
	}
}

DWORD delta_time = 0;
DWORD current_time = 0;
DWORD prev_time = 0;

bool Server_Run() {
	printf("Server_Run \n");

	prev_time = timeGetTime();
	for (;;) {
		NetWork_Process();

		current_time = timeGetTime();
		delta_time += current_time - prev_time;
		Update_Logic();
		prev_time = current_time;
	}

	return true;
}

bool Server_Close() {
	printf("Server_Close");

	WSACleanup();

	return true;
}

void Accept_Proc(Session* user_info) {
	// Accept & UserInfo Set
	SOCKET tmp_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_size);
	if (tmp_sock == INVALID_SOCKET) {
		auto erro_num = WSAGetLastError();
		switch (erro_num) {
		default:
			CRASH();
			break;
		}
	}
	user_info->Set(tmp_sock, ntohl(client_addr.sin_addr.s_addr), ntohs(client_addr.sin_port), Get_Session_No());

	// 디버그 코드
	printf("[SOCKET : %d, id : %d] ACCEPT \n", (int)user_info->sock, user_info->id);

	// Send to Accept User
		// Accept 캐릭터 할당 패킷
	PACKET_HEADER my_character_header;
	my_character_header.code = 0x89;
	my_character_header.size = sizeof(packet_sc_create_my_character);
	my_character_header.type = PACKET_SC_CREATE_MY_CHARACTER;

	packet_sc_create_my_character my_character_packet;
	my_character_packet.id = user_info->id;
	my_character_packet.dir = user_info->dir;
	my_character_packet.x = user_info->x;
	my_character_packet.y = user_info->y;
	my_character_packet.hp = user_info->hp;

	auto ret1 = Unicast(user_info, (char*)&my_character_header, sizeof(my_character_header));
	auto ret2 = Unicast(user_info, (char*)&my_character_packet, sizeof(my_character_packet));

	// Send to Accept User
		// Other 캐릭터 생성 패킷
	for (Session& other_user : sessions) {
		if (other_user.sock == INVALID_SOCKET)
			continue;
		if (other_user.sock == user_info->sock)
			continue;
		if (other_user.disconnect_flag)
			continue;
		
		PACKET_HEADER other_character_header;
		other_character_header.code = 0x89;
		other_character_header.size = sizeof(packet_sc_create_other_character);
		other_character_header.type = PACKET_SC_CREATE_OTHER_CHARACTER;

		packet_sc_create_other_character other_character_packet;
		other_character_packet.id = other_user.id;
		other_character_packet.dir = other_user.dir;
		other_character_packet.x = other_user.x;
		other_character_packet.y = other_user.y;
		other_character_packet.hp = other_user.hp;

		Unicast(user_info, (char*)&other_character_header, sizeof(other_character_header));
		Unicast(user_info, (char*)&other_character_packet, sizeof(other_character_packet));
	}

	// Send to Other Users // Broadcast
		// Accept 캐릭터 생성 패킷
	PACKET_HEADER accept_character_header;
	accept_character_header.code = 0x89;
	accept_character_header.size = sizeof(packet_sc_create_other_character);
	accept_character_header.type = PACKET_SC_CREATE_OTHER_CHARACTER;

	packet_sc_create_other_character accept_character_packet;
	accept_character_packet.id = user_info->id;
	accept_character_packet.dir = user_info->dir;
	accept_character_packet.x = user_info->x;
	accept_character_packet.y = user_info->y;
	accept_character_packet.hp = user_info->hp;

	char packet_buf[MAX_PACKET_SIZE];
	memmove(packet_buf, &accept_character_header, sizeof(accept_character_header));
	memmove(packet_buf + sizeof(accept_character_header), &accept_character_packet, sizeof(accept_character_packet));

	vector<Session*> exclude_users;
	exclude_users.emplace_back(user_info);
	Broadcast(&exclude_users, packet_buf, sizeof(accept_character_header) + sizeof(accept_character_packet));
}

void Close_Proc(Session* user_info) {
	emplace_disconnect_queue(user_info);

	PACKET_HEADER delete_header;
	delete_header.code = 0x89;
	delete_header.size = sizeof(packet_sc_delete_character);
	delete_header.type = PACKET_SC_DELETE_CHARACTER;

	packet_sc_delete_character delete_packet;
	delete_packet.id = user_info->id;

	char packet_buf[MAX_PACKET_SIZE];
	memmove(packet_buf, &delete_header, sizeof(delete_header));
	memmove(packet_buf + sizeof(delete_header), &delete_packet, sizeof(delete_packet));

	Broadcast(nullptr, packet_buf, sizeof(delete_header) + sizeof(delete_packet));
}

void Recv_Proc(Session* user_info) {
	if (user_info->recv_buf.GetUseSize() <= sizeof(PACKET_HEADER))
		return;

	PACKET_HEADER header;
	auto ret_peek = user_info->recv_buf.Peek((char*)&header, sizeof(header));

	if (header.code != 0x89) {
		emplace_disconnect_queue(user_info);
		return;
	}
	if (user_info->recv_buf.GetUseSize() < sizeof(PACKET_HEADER) + header.size)
		return;

	user_info->recv_buf.MoveFront(ret_peek);

	switch (header.type) {
	case PACKET_CS_MOVE_START: {
		printf("[SOCKET : %d, id : %d] PACKET_CS_MOVE_START \n", (int)user_info->sock, user_info->id);

		// 수신 패킷 분석 부
		packet_cs_move_start cs_move_start_packet;
		auto ret = user_info->recv_buf.Dequeue((char*)&cs_move_start_packet, sizeof(cs_move_start_packet));
		if (ret != sizeof(cs_move_start_packet)) {
			CRASH();
		}

		user_info->dir = cs_move_start_packet.dir;
		user_info->x = cs_move_start_packet.x;
		user_info->y = cs_move_start_packet.y;

		Push_MoveUsers(user_info);

		// 패킷 송신 부
		PACKET_HEADER sc_move_start_header;
		sc_move_start_header.code = 0x89;
		sc_move_start_header.size = sizeof(packet_sc_move_start);
		sc_move_start_header.type = PACKET_SC_MOVE_START;

		packet_sc_move_start sc_move_start_packet;
		sc_move_start_packet.dir = user_info->dir;
		sc_move_start_packet.id = user_info->id;
		sc_move_start_packet.x = user_info->x;
		sc_move_start_packet.y = user_info->y;

		char packet_buf[MAX_PACKET_SIZE];
		memmove(packet_buf, &sc_move_start_header, sizeof(sc_move_start_header));
		memmove(packet_buf + sizeof(sc_move_start_header), &sc_move_start_packet, sizeof(sc_move_start_packet));

		vector<Session*> exclude_users;
		exclude_users.emplace_back(user_info);
		Broadcast(&exclude_users, packet_buf, sizeof(sc_move_start_header) + sizeof(sc_move_start_packet));
	}
							 break;

	case PACKET_CS_MOVE_STOP: {
		printf("[SOCKET : %d, id : %d] PACKET_CS_MOVE_STOP \n", (int)user_info->sock, user_info->id);

		// 수신 패킷 분석 부
		packet_cs_move_stop move_stop_packet;
		auto ret = user_info->recv_buf.Dequeue((char*)&move_stop_packet, sizeof(move_stop_packet));
		if (ret != sizeof(move_stop_packet)) {
			CRASH();
		}

		user_info->dir = move_stop_packet.dir;
		user_info->x = move_stop_packet.x;
		user_info->y = move_stop_packet.y;

		Pop_MoveUsers(user_info);

		// 패킷 송신 부
		PACKET_HEADER sc_move_stop_header;
		sc_move_stop_header.code = 0x89;
		sc_move_stop_header.size = sizeof(packet_sc_move_stop);
		sc_move_stop_header.type = PACKET_SC_MOVE_STOP;

		packet_sc_move_stop sc_move_stop_packet;
		sc_move_stop_packet.dir = user_info->dir;
		sc_move_stop_packet.id = user_info->id;
		sc_move_stop_packet.x = user_info->x;
		sc_move_stop_packet.y = user_info->y;

		char packet_buf[MAX_PACKET_SIZE];
		memmove(packet_buf, &sc_move_stop_header, sizeof(sc_move_stop_header));
		memmove(packet_buf + sizeof(sc_move_stop_header), &sc_move_stop_packet, sizeof(sc_move_stop_packet));

		vector<Session*> exclude_users;
		exclude_users.emplace_back(user_info);
		Broadcast(&exclude_users, packet_buf, sizeof(sc_move_stop_header) + sizeof(sc_move_stop_packet));
	}
							break;

	case PACKET_CS_ATTACK1: {
		printf("[SOCKET : %d, id : %d] PACKET_CS_ATTACK1 \n", (int)user_info->sock, user_info->id);

		// 수신 패킷 분석 부
		packet_cs_attack1 cs_attack1_packet;
		auto ret = user_info->recv_buf.Dequeue((char*)&cs_attack1_packet, sizeof(cs_attack1_packet));
		if (ret != sizeof(cs_attack1_packet)) {
			CRASH();
		}

		user_info->dir = cs_attack1_packet.dir;
		user_info->x = cs_attack1_packet.x;
		user_info->y = cs_attack1_packet.y;

		Pop_MoveUsers(user_info);

		// 패킷 송신 부
			// 데미지 패킷 브로드 캐스트
		for (Session& other_user : sessions) {
			if (other_user.sock == INVALID_SOCKET)
				continue;
			if (other_user.disconnect_flag)
				continue;
			if (other_user.sock == user_info->sock)
				continue;

			// 왼쪽 보는 경우
			if (user_info->dir == PACKET_MOVE_DIR_LL || user_info->dir == PACKET_MOVE_DIR_LU || user_info->dir == PACKET_MOVE_DIR_LD) {
				if (user_info->x < other_user.x)
					continue;
				if (abs(user_info->y - other_user.y) > ATTACK1_RANGE_Y)
					continue;
				if (other_user.x + ATTACK1_RANGE_X  < user_info->x)
					continue;
			}
			// 오른쪽 보는 경우
			else if (user_info->dir == PACKET_MOVE_DIR_RR || user_info->dir == PACKET_MOVE_DIR_RU || user_info->dir == PACKET_MOVE_DIR_RD) {
				if (other_user.x < user_info->x )
					continue;
				if (abs(user_info->y - other_user.y) > ATTACK1_RANGE_Y)
					continue;
				if (user_info->x + ATTACK1_RANGE_X < other_user.y)
					continue;
			}

			other_user.hp -= ATTACK1_DAMAGE;
			if (other_user.hp > 0) {
				PACKET_HEADER sc_damage_header;
				sc_damage_header.code = 0x89;
				sc_damage_header.size = sizeof(packet_sc_damage);
				sc_damage_header.type = PACKET_SC_DAMAGE;

				packet_sc_damage sc_damage_packet;
				sc_damage_packet.attacker_id = user_info->id;
				sc_damage_packet.damaged_id = other_user.id;
				sc_damage_packet.hp = other_user.hp;

				char packet_buf[MAX_PACKET_SIZE];
				memmove(packet_buf, &sc_damage_header, sizeof(sc_damage_header));
				memmove(packet_buf + sizeof(sc_damage_header), &sc_damage_packet, sizeof(sc_damage_packet));

				Broadcast(nullptr, packet_buf, sizeof(sc_damage_header) + sizeof(sc_damage_packet));
			}
			else {
				Close_Proc(&other_user);
			}

			break;
		}

			// 공격 패킷 브로드 캐스트
		PACKET_HEADER sc_attack1_header;
		sc_attack1_header.code = 0x89;
		sc_attack1_header.size = sizeof(packet_sc_attack1);
		sc_attack1_header.type = PACKET_SC_ATTACK1;

		packet_sc_attack1 sc_attack1_packet;
		sc_attack1_packet.dir = user_info->dir;
		sc_attack1_packet.id = user_info->id;
		sc_attack1_packet.x = user_info->x;
		sc_attack1_packet.y = user_info->y;

		char packet_buf[MAX_PACKET_SIZE];
		memmove(packet_buf, &sc_attack1_header, sizeof(sc_attack1_header));
		memmove(packet_buf + sizeof(sc_attack1_header), &sc_attack1_packet, sizeof(sc_attack1_packet));

		vector<Session*> exclude_users;
		exclude_users.emplace_back(user_info);
		Broadcast(&exclude_users, packet_buf, sizeof(sc_attack1_header) + sizeof(sc_attack1_packet));
	}
						  break;

	case PACKET_CS_ATTACK2: {
		printf("[SOCKET : %d, id : %d] PACKET_CS_ATTACK2 \n", (int)user_info->sock, user_info->id);

		// 수신 패킷 분석 부
		packet_cs_attack2 cs_attack2_packet;
		auto ret = user_info->recv_buf.Dequeue((char*)&cs_attack2_packet, sizeof(cs_attack2_packet));
		if (ret != sizeof(cs_attack2_packet)) {
			CRASH();
		}

		user_info->dir = cs_attack2_packet.dir;
		user_info->x = cs_attack2_packet.x;
		user_info->y = cs_attack2_packet.y;

		Pop_MoveUsers(user_info);

		// 패킷 송신 부
			// 데미지 패킷 브로드 캐스트
		for (Session& other_user : sessions) {
			if (other_user.sock == INVALID_SOCKET)
				continue;
			if (other_user.disconnect_flag)
				continue;
			if (other_user.sock == user_info->sock)
				continue;

			// 왼쪽 보는 경우
			if (user_info->dir == PACKET_MOVE_DIR_LL || user_info->dir == PACKET_MOVE_DIR_LU || user_info->dir == PACKET_MOVE_DIR_LD) {
				if (user_info->x < other_user.x)
					continue;
				if (abs(user_info->y - other_user.y) > ATTACK2_RANGE_Y)
					continue;
				if (other_user.x + ATTACK2_RANGE_X < user_info->x)
					continue;
			}
			// 오른쪽 보는 경우
			else if (user_info->dir == PACKET_MOVE_DIR_RR || user_info->dir == PACKET_MOVE_DIR_RU || user_info->dir == PACKET_MOVE_DIR_RD) {
				if (other_user.x < user_info->x)
					continue;
				if (abs(user_info->y - other_user.y) > ATTACK2_RANGE_Y)
					continue;
				if (user_info->x + ATTACK2_RANGE_X < other_user.y)
					continue;
			}


			other_user.hp -= ATTACK2_DAMAGE;
			if (other_user.hp > 0) {
				PACKET_HEADER sc_damage_header;
				sc_damage_header.code = 0x89;
				sc_damage_header.size = sizeof(packet_sc_damage);
				sc_damage_header.type = PACKET_SC_DAMAGE;

				packet_sc_damage sc_damage_packet;
				sc_damage_packet.attacker_id = user_info->id;
				sc_damage_packet.damaged_id = other_user.id;
				sc_damage_packet.hp = other_user.hp;

				char packet_buf[MAX_PACKET_SIZE];
				memmove(packet_buf, &sc_damage_header, sizeof(sc_damage_header));
				memmove(packet_buf + sizeof(sc_damage_header), &sc_damage_packet, sizeof(sc_damage_packet));

				Broadcast(nullptr, packet_buf, sizeof(sc_damage_header) + sizeof(sc_damage_packet));
			}
			else {
				Close_Proc(&other_user);
			}

			break;
		}

		// 공격 패킷 브로드 캐스트
		PACKET_HEADER sc_attack2_header;
		sc_attack2_header.code = 0x89;
		sc_attack2_header.size = sizeof(packet_sc_attack2);
		sc_attack2_header.type = PACKET_SC_ATTACK2;

		packet_sc_attack2 sc_attack2_packet;
		sc_attack2_packet.dir = user_info->dir;
		sc_attack2_packet.id = user_info->id;
		sc_attack2_packet.x = user_info->x;
		sc_attack2_packet.y = user_info->y;

		char packet_buf[MAX_PACKET_SIZE];
		memmove(packet_buf, &sc_attack2_header, sizeof(sc_attack2_header));
		memmove(packet_buf + sizeof(sc_attack2_header), &sc_attack2_packet, sizeof(sc_attack2_packet));

		vector<Session*> exclude_users;
		exclude_users.emplace_back(user_info);
		Broadcast(&exclude_users, packet_buf, sizeof(sc_attack2_header) + sizeof(sc_attack2_packet));
	}
						  break;

	case PACKET_CS_ATTACK3: {
		printf("[SOCKET : %d, id : %d] PACKET_CS_ATTACK3 \n", (int)user_info->sock, user_info->id);

		// 수신 패킷 분석 부
		packet_cs_attack3 cs_attack3_packet;
		auto ret = user_info->recv_buf.Dequeue((char*)&cs_attack3_packet, sizeof(cs_attack3_packet));
		if (ret != sizeof(cs_attack3_packet)) {
			CRASH();
		}

		user_info->dir = cs_attack3_packet.dir;
		user_info->x = cs_attack3_packet.x;
		user_info->y = cs_attack3_packet.y;

		Pop_MoveUsers(user_info);

		// 패킷 송신 부
			// 데미지 패킷 브로드 캐스트
		for (Session& other_user : sessions) {
			if (other_user.sock == INVALID_SOCKET)
				continue;
			if (other_user.disconnect_flag)
				continue;
			if (other_user.sock == user_info->sock)
				continue;

			// 왼쪽 보는 경우
			if (user_info->dir == PACKET_MOVE_DIR_LL || user_info->dir == PACKET_MOVE_DIR_LU || user_info->dir == PACKET_MOVE_DIR_LD) {
				if (user_info->x < other_user.x)
					continue;
				if (abs(user_info->y - other_user.y) > ATTACK3_RANGE_Y)
					continue;
				if (other_user.x + ATTACK3_RANGE_X < user_info->x)
					continue;
			}
			// 오른쪽 보는 경우
			else if (user_info->dir == PACKET_MOVE_DIR_RR || user_info->dir == PACKET_MOVE_DIR_RU || user_info->dir == PACKET_MOVE_DIR_RD) {
				if (other_user.x < user_info->x)
					continue;
				if (abs(user_info->y - other_user.y) > ATTACK3_RANGE_Y)
					continue;
				if (user_info->x + ATTACK3_RANGE_X < other_user.y)
					continue;
			}


			other_user.hp -= ATTACK3_DAMAGE;
			if (other_user.hp > 0) {
				PACKET_HEADER sc_damage_header;
				sc_damage_header.code = 0x89;
				sc_damage_header.size = sizeof(packet_sc_damage);
				sc_damage_header.type = PACKET_SC_DAMAGE;

				packet_sc_damage sc_damage_packet;
				sc_damage_packet.attacker_id = user_info->id;
				sc_damage_packet.damaged_id = other_user.id;
				sc_damage_packet.hp = other_user.hp;

				char packet_buf[MAX_PACKET_SIZE];
				memmove(packet_buf, &sc_damage_header, sizeof(sc_damage_header));
				memmove(packet_buf + sizeof(sc_damage_header), &sc_damage_packet, sizeof(sc_damage_packet));

				Broadcast(nullptr, packet_buf, sizeof(sc_damage_header) + sizeof(sc_damage_packet));
			}
			else {
				Close_Proc(&other_user);
			}

			break;
		}

		// 공격 패킷 브로드 캐스트
		PACKET_HEADER sc_attack3_header;
		sc_attack3_header.code = 0x89;
		sc_attack3_header.size = sizeof(packet_sc_attack3);
		sc_attack3_header.type = PACKET_SC_ATTACK3;

		packet_sc_attack3 sc_attack3_packet;
		sc_attack3_packet.dir = user_info->dir;
		sc_attack3_packet.id = user_info->id;
		sc_attack3_packet.x = user_info->x;
		sc_attack3_packet.y = user_info->y;

		char packet_buf[MAX_PACKET_SIZE];
		memmove(packet_buf, &sc_attack3_header, sizeof(sc_attack3_header));
		memmove(packet_buf + sizeof(sc_attack3_header), &sc_attack3_packet, sizeof(sc_attack3_packet));

		vector<Session*> exclude_users;
		exclude_users.emplace_back(user_info);
		Broadcast(&exclude_users, packet_buf, sizeof(sc_attack3_header) + sizeof(sc_attack3_packet));
	}
						  break;

	case PACKET_CS_SYNC: {
		printf("[SOCKET : %d, id : %d] PACKET_CS_SYNC \n", (int)user_info->sock, user_info->id);
		packet_cs_sync sync_packet;
		auto ret = user_info->recv_buf.Dequeue((char*)&sync_packet, sizeof(sync_packet));
		if (ret != sizeof(sync_packet)) {
			CRASH();
		}

		auto x = sync_packet.x;
		auto y = sync_packet.y;
	}
					   break;

	default:
		CRASH();
		break;
	}
}

void Send_Proc(Session* user_info) {
	char tmp_buf[BUF_SIZE];
	auto ret_peek = user_info->send_buf.Peek(tmp_buf, BUF_SIZE);
	auto ret_send = send(user_info->sock, tmp_buf, ret_peek, NULL);
	if (ret_send == SOCKET_ERROR) {
		auto erro_num = WSAGetLastError();
		switch (erro_num) {
		case WSAECONNRESET:
			emplace_disconnect_queue(user_info);
			break;
		default:
			CRASH();
			break;
		}
	}
	user_info->send_buf.MoveFront(ret_send);
}

void Update_Logic(){
	if (delta_time < FRAME_TIME)
		return;

	delta_time -= FRAME_TIME;
	
	for (auto& p_move_user : move_users) {
		if (p_move_user == nullptr)
			continue;
		if (p_move_user->sock == INVALID_SOCKET || p_move_user->disconnect_flag) {
			Pop_MoveUsers(p_move_user);
			continue;
		}
		if (p_move_user->x <= RANGE_MOVE_LEFT || RANGE_MOVE_RIGHT <= p_move_user->x)
			continue;
		if (p_move_user->y <= RANGE_MOVE_TOP || RANGE_MOVE_BOTTOM <= p_move_user->y)
			continue;

		switch (p_move_user->dir) {
		case PACKET_MOVE_DIR_LL:
			p_move_user->x -= 3;
			if (p_move_user->x < RANGE_MOVE_LEFT) {
				p_move_user->x = RANGE_MOVE_LEFT;
				break;
			}
			break;

		case PACKET_MOVE_DIR_RR:
			p_move_user->x += 3;
			if (p_move_user->x > RANGE_MOVE_RIGHT) {
				p_move_user->x = RANGE_MOVE_RIGHT;
				break;
			}
			break;

		case PACKET_MOVE_DIR_UU:
			p_move_user->y -= 2;
			if (p_move_user->y < RANGE_MOVE_TOP) {
				p_move_user->y = RANGE_MOVE_TOP;
				break;
			}
			break;

		case PACKET_MOVE_DIR_DD:
			p_move_user->y += 2;
			if (p_move_user->y > RANGE_MOVE_BOTTOM) {
				p_move_user->y = RANGE_MOVE_BOTTOM;
				break;
			}
			break;

		case PACKET_MOVE_DIR_LU:
			p_move_user->x -= 3;
			if (p_move_user->x < RANGE_MOVE_LEFT) {
				p_move_user->x = RANGE_MOVE_LEFT;
				break;
			}
			p_move_user->y -= 2;
			if (p_move_user->y < RANGE_MOVE_TOP) {
				p_move_user->y = RANGE_MOVE_TOP;
				break;
			}
			break;

		case PACKET_MOVE_DIR_LD:
			p_move_user->x -= 3;
			if (p_move_user->x < RANGE_MOVE_LEFT) {
				p_move_user->x = RANGE_MOVE_LEFT;
				break;
			}
			p_move_user->y += 2;
			if (p_move_user->y > RANGE_MOVE_BOTTOM) {
				p_move_user->y = RANGE_MOVE_BOTTOM;
				break;
			}
			break;

		case PACKET_MOVE_DIR_RU:
			p_move_user->x += 3;
			if (p_move_user->x > RANGE_MOVE_RIGHT) {
				p_move_user->x = RANGE_MOVE_RIGHT;
				break;
			}
			p_move_user->y -= 2;
			if (p_move_user->y < RANGE_MOVE_TOP) {
				p_move_user->y = RANGE_MOVE_TOP;
				break;
			}
			break;

		case PACKET_MOVE_DIR_RD:
			p_move_user->x += 3;
			if (p_move_user->x > RANGE_MOVE_RIGHT) {
				p_move_user->x = RANGE_MOVE_RIGHT;
				break;
			}
			p_move_user->y += 2;
			if (p_move_user->y > RANGE_MOVE_BOTTOM) {
				p_move_user->y = RANGE_MOVE_BOTTOM;
				break;
			}
			break;
		}

		printf("[SOCKET : %d, id : %d] MOVE (%d, %d) \n", (int)p_move_user->sock, p_move_user->id, p_move_user->x, p_move_user->y);
	}
}


int Unicast(Session* user_info, const char* packet, const unsigned short packet_size) {
	auto ret = user_info->send_buf.Enqueue(packet, packet_size);
	if (ret != packet_size) {
		if (user_info->send_buf.Full()) {
			emplace_disconnect_queue(user_info);
		}
		else {
			CRASH();
		}
	}

	return ret;
}

void Broadcast(vector<Session*>* exclude_users, const char* packet, const unsigned short packet_size) {
	for (Session& user_info : sessions) {
		if (user_info.sock == INVALID_SOCKET)
			continue;

		if (exclude_users != nullptr) {
			for (int i = 0; i < (*exclude_users).size(); i++) {
				if (*(SOCKET*)(*exclude_users)[i] == user_info.sock)
					goto next_UserInfo;
			}
		}

		Unicast(&user_info, packet, packet_size);
	next_UserInfo:;
	}
}

void Unicast_packet_sc_create_my_character(
	Session* user_info,
	unsigned int id,
	unsigned char dir,
	unsigned short x,
	unsigned short y,
	char hp
) {
	PACKET_HEADER header;
	header.code = 0x89;
	header.size = sizeof(packet_sc_create_my_character);
	header.type = PACKET_SC_CREATE_MY_CHARACTER;

	packet_sc_create_my_character packet;
	packet.id = id;
	packet.dir = dir;
	packet.x = x;
	packet.y = y;
	packet.hp = hp;

	Unicast(user_info, (char*)&header, sizeof(header));
	Unicast(user_info, (char*)&packet, sizeof(packet));
}