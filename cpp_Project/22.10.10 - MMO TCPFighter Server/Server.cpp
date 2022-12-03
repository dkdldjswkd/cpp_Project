#include "stdafx.h"
#include "Server.h"
#include "Character.h"
#include "MakePacket.h"
#include "Session.h"
#include "Sector.h"
#include "Define.h"
#include "Protocol.h"
#include "Log.h"
#pragma comment(lib, "../../J_LIB/ProtocolBuffer/ProtocolBuffer.lib")

using namespace std;

SOCKET listen_sock = INVALID_SOCKET;
sockaddr_in server_addr;
unordered_map<SOCKET, Session*>			session_map;
unordered_map<SESSION_ID, Character*>	character_map;

queue<Session*> disconnect_queue;

// Object_Pool
ObjectPool<ProtocolBuffer>	pool_protocolBuf;
ObjectPool<Session>			pool_session;
ObjectPool<Character>		pool_character;

bool Input_DisconnectQ(Session* p_session) {
	if (p_session->flag_disconnect)
		return false;

	disconnect_count++;
	p_session->flag_disconnect = true;
	disconnect_queue.push(p_session);
	return true;
}

bool Input_DisconnectQ(Session* p_session, const char* str) {
	auto ret = Input_DisconnectQ(p_session);
	//printf("%s \n", str);
	//Log::Debug("Disconnect Reason : %s, session_id(%d) ", str, p_session->session_id);
	return ret;
}

void Network::Disconnect_Session() {
	int queue_size = disconnect_queue.size();

	for (int i = 0; i < queue_size; i++) {
		Session* p_disconnect_session = disconnect_queue.front();

		auto p_character = character_map.find(p_disconnect_session->session_id)->second;

		//------------------------------
		// Sectors���� ����(���� ��Ŷ Send), closesocket
		//------------------------------
		SectorFunc::Erase_Sectors(p_character);
		SCPacket::Disconnect_Character(p_character);
		character_map.erase(p_character->session_id);
		session_map.erase(p_disconnect_session->sock);
		closesocket(p_disconnect_session->sock);

		pool_character.Free(p_character);			/* ī���� */ character_count--;
		pool_session.Free(p_disconnect_session);	/* ī���� */ session_count--;
		disconnect_queue.pop();
	}
}

bool Network::StartUp() {
	printf("Network::Startup() \n");

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		throw;

	//------------------------------
	// socket()
	//------------------------------
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		throw;

	//------------------------------
	// Set Addr
	//------------------------------
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(NETWORK_PORT);

	//------------------------------
	// bind()
	//------------------------------
	if (SOCKET_ERROR == bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)))
		throw;

	//------------------------------
	// set sock opt
	//------------------------------
	LINGER opt = { 1, 0 };
	if (0 != setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt)))
		throw;

	//------------------------------
	// set I/O mode
	//------------------------------
	u_long io_mode = 1;
	if (0 != ioctlsocket(listen_sock, FIONBIO, &io_mode))
		throw;

	//------------------------------
	// listen()
	//------------------------------
	if (SOCKET_ERROR == listen(listen_sock, SOMAXCONN))
		throw;

	return true;
}

bool Network::CleanUp() {
	WSACleanup();

	return true;
}

void Network::Process_NetIO() {
	//--------------------------------
	// ���� ����
	//--------------------------------
	Session* p_session;

	SOCKET sessionTable_sock[FD_SETSIZE] = { INVALID_SOCKET, };
	int sock_count = 0;

	FD_SET read_set;
	FD_SET write_set;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	//--------------------------------
	// ���� ���� �ֱ�
	//--------------------------------
	FD_SET(listen_sock, &read_set);
	sessionTable_sock[sock_count++] = listen_sock;

	//--------------------------------
	// �������� ��� ȣ��Ʈ�� socket�� üũ
	//--------------------------------
	for (auto session_iter = session_map.begin(); session_iter != session_map.end(); session_iter++) {
		p_session = session_iter->second;
		sessionTable_sock[sock_count] = p_session->sock;

		//--------------------------------
		// read/write set ���
		//--------------------------------
		FD_SET(p_session->sock, &read_set);
		if (p_session->send_buf.Get_UseSize() > 0)
			FD_SET(p_session->sock, &write_set);
		sock_count++;

		//--------------------------------
		// select �ִ�ġ ����, select ȣ�� �� ����
		//--------------------------------
		if (FD_SETSIZE <= sock_count) {
			Select_Socket(sessionTable_sock, &read_set, &write_set);

			FD_ZERO(&read_set);
			FD_ZERO(&write_set);
			memset(sessionTable_sock, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			FD_SET(listen_sock, &read_set);
			sock_count = 1;
		}
	}

	if (sock_count > 0) {
		Select_Socket(sessionTable_sock, &read_set, &write_set);
	}
}

void Network::Select_Socket(SOCKET* p_SocketTable, FD_SET* p_ReadSet, FD_SET* p_WriteSet) {
	bool bProcFlag;

	timeval	t;
	t.tv_sec = 0;
	t.tv_usec = 0;
	auto ret_select = select(0, p_ReadSet, p_WriteSet, 0, &t);

	if (0 < ret_select) {
		for (int i = 0; 0 < ret_select && i < FD_SETSIZE; i++) {
			bProcFlag = true;
			if (p_SocketTable[i] == INVALID_SOCKET)
				continue;

			if (FD_ISSET(p_SocketTable[i], p_WriteSet)) {
				--ret_select;
				bProcFlag = Proc_Send(p_SocketTable[i]);
			}

			if (FD_ISSET(p_SocketTable[i], p_ReadSet)) {
				--ret_select;

				if (bProcFlag) {
					if (p_SocketTable[i] == listen_sock) {
						Proc_Accept();
					}
					else if (p_SocketTable[i] != listen_sock) {
						Proc_Recv(p_SocketTable[i]);
					}
				}
			}
		}
	}
	//else if (ret_select == SOCKET_ERROR) {
	//	//Error(L"select() error");
	//}
}

void Network::Proc_Recv(SOCKET sock) {
	Session* p_session;

	p_session = session_map.find(sock)->second;
	p_session->last_recvTime = timeGetTime();

	// Recv Buf FULL *ũ��Ƽ��
	if (p_session->recv_buf.Full()) {
		Log::Error("RECV BUF FULL : session_id(%d) ", p_session->session_id);
		Input_DisconnectQ(p_session, "Recv buf full");
	}
	auto recv_size = recv(p_session->sock, p_session->recv_buf.Get_WritePos(), p_session->recv_buf.Direct_EnqueueSize(), 0);	// (ó���õ��ϴ� �ڵ� ���� ����)

	// FIN ����
	if (recv_size == 0) {
		fin_count++;
		Input_DisconnectQ(p_session, "recv size == 0");
		return;
	}
	// SOCKET_ERROR
	if (SOCKET_ERROR == recv_size) {
		int err_no = WSAGetLastError();

		switch (err_no)		{
			case 10053:
			case 10054:
				break;

			default:
				Log::Error("[Recv ERORR] %d", err_no);
		}

		Input_DisconnectQ(p_session, "recv SOCKET_ERROR");
		return;
	}

	if (0 < recv_size) {
		p_session->recv_buf.Move_Rear(recv_size);
		for (;;) {
			auto ret = Complete_RecvPacket(p_session);

			// �� �̻� ó���� ��Ŷ ����
			if (1 == ret)
				break;

			// ��Ŷ ó�� ����
			if (-1 == ret) {
				Input_DisconnectQ(p_session, "Complete_RecvPacket == -1");
				break;
			}
		}
	}
}

extern int total_loop;
bool Network::Proc_Send(SOCKET sock) {
	char tmp_buf[BUF_SIZE];
	Session* p_session = session_map.find(sock)->second;

	auto ret_send = send(sock, p_session->send_buf.Get_ReadPos(), p_session->send_buf.Direct_DequeueSize(), NULL); // (ó���õ��ϴ� �ڵ� ���� ����)

	// Send Error
	if (ret_send == SOCKET_ERROR) {
		auto err_no = WSAGetLastError();

		switch (err_no) {
			case 10053:
			case 10054:
				break;

			default:
				Log::Error("[Send ERORR] %d", err_no);
		}

		Input_DisconnectQ(p_session, "Send SOCKET_ERROR");
		return false;
	}

	// Send ����
	p_session->send_buf.Move_Front(ret_send);
	return true;
}

extern int total_loop;
bool Network::Proc_Accept() {
	sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	//------------------------------
	// Accept
	//------------------------------
	auto accept_sock = accept(listen_sock, (sockaddr*)&client_addr, &addr_len);
	if (accept_sock == INVALID_SOCKET) {
		int err_no = WSAGetLastError();

		switch (err_no)
		{
			case 10053:
			case 10054:
				break;

		default:
			Log::Error("[Accept ERORR] %d", err_no);
		}
		
		return false;
	}

	//------------------------------
	// ����, ĳ���� �ڷᱸ�� ���
	//------------------------------
	Session* p_accept_session = pool_session.Alloc(); /* ī���� */ session_count++;
	p_accept_session->Clear(total_loop);
	p_accept_session->Set(accept_sock, Get_SessionID(), timeGetTime());
	session_map.insert({ accept_sock, p_accept_session });

	Character* p_accept_character = pool_character.Alloc(); /* ī���� */ character_count++;
	p_accept_character->Clear();
	UINT16 x = rand() % RANGE_MOVE_RIGHT;
	UINT16 y = rand() % RANGE_MOVE_BOTTOM;
	p_accept_character->Set(p_accept_session, x, y);
	character_map.insert({ p_accept_session->session_id, p_accept_character });

	//------------------------------
	// Sectors ���
	//------------------------------
	SectorFunc::Insert_Sector(p_accept_character);

	//------------------------------
	// ��Ŷ ����
	//------------------------------
	SCPacket::CREATE_MY_CHARACTER(p_accept_character);

//	// �����
//	if (DEBUF_LEVEL >= 0) {
//		wchar_t wcsstr[100];
//#pragma warning(suppress : 4996)
//		swprintf(wcsstr, L"[Accept] id / socket (%u, %lld), pos(%d, %d) \n", p_accept_session->session_id, p_accept_session->sock, p_accept_character->x, p_accept_character->y);
//		OutputDebugString(wcsstr);
//	}

	return true;
}

void SendPacket_Unicast(Session* p_session, ProtocolBuffer* p_packet) {
	auto packet_size = p_packet->Get_UseSize();
	auto enqueue_size = p_session->send_buf.Enqueue(p_packet->Get_readPos(), packet_size);

	// ���� ���� ���� ��
	if (enqueue_size != packet_size) {
		if (p_session->send_buf.Full()) {
			Input_DisconnectQ(p_session, "Send buf full");
		}
		//else // ����� �ڵ�
		//	throw;
	}
}

void SendPacket_SectorOne(int sector_x, int sector_y, ProtocolBuffer* p_packet, Session* p_except_session) {
	//printf("SendPacket_SectorOne(%d, %d) \n", sector_x, sector_y);

	for (auto sectors_iter = sectors[sector_y][sector_x].begin(); sectors_iter != sectors[sector_y][sector_x].end(); sectors_iter++) {
		Session* p_session = (*sectors_iter)->p_session;
		if ((*sectors_iter)->p_session == p_except_session) continue;

		SendPacket_Unicast(p_session, p_packet);
	}
}

void SendPacket_Around(SectorAround* p_sector_around, ProtocolBuffer* p_packet, Session* p_except_session) {
	for (int i = 0; i < p_sector_around->count; i++) {
		SendPacket_SectorOne(p_sector_around->around[i].x, p_sector_around->around[i].y, p_packet, p_except_session);
	}
}

// 1. ���� ���ǿ��� ���� ��Ŷ Send
// 2. ���� ĳ���Ϳ��� �ֺ� ĳ���� ����, �̵� ��Ŷ Send
// 3. �ֺ� ĳ���Ϳ��� ���� ĳ���� ���� Send
void SCPacket::CREATE_MY_CHARACTER(Character* p_character) {
	//------------------------------
	// (����)���� ��Ŷ Send
	//------------------------------
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();
	sc_packet.Clear();
	MakePacket_CREATE_MY_CHARACTER(&sc_packet, p_character->session_id, p_character->dir, p_character->x, p_character->y, p_character->hp);
	SendPacket_Unicast(p_character->p_session, &sc_packet);

	//------------------------------
	// Cur Sector ĳ���͵鿡�� (����)ĳ���� ���� ��Ŷ
	//------------------------------
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	sc_packet.Clear();
	MakePacket_CREATE_OTHER_CHARACTER(&sc_packet, p_character->session_id, p_character->dir, p_character->x, p_character->y, p_character->hp);
	SendPacket_Around(&sector_around, &sc_packet, p_character->p_session);

	//------------------------------
	// (����)ĳ���Ϳ��� Cur Sector ĳ���͵� ����, �̵� ��Ŷ
	//------------------------------
	for (int i = 0; i < sector_around.count; i++) {
		const auto& sector = sectors[sector_around.around[i].y][sector_around.around[i].x];
		for (auto iter = sector.begin(); iter != sector.end(); iter++) {
			if (*iter == p_character) continue;
			const auto& p_other_character = *iter;

			sc_packet.Clear();
			MakePacket_CREATE_OTHER_CHARACTER(&sc_packet, p_other_character->session_id, p_other_character->dir, p_other_character->x, p_other_character->y, p_other_character->hp);
			if (p_other_character->action != ACTION::STOP)
				MakePacket_MOVE_START(&sc_packet, p_other_character->session_id, p_other_character->action, p_other_character->x, p_other_character->y);
			SendPacket_Unicast(p_character->p_session, &sc_packet);
		}
	}

	pool_protocolBuf.Free(&sc_packet);
}



bool Proc_Packet(Session* p_session, unsigned char packet_type, ProtocolBuffer* cs_payload) {
	switch (packet_type) {
		case dfPACKET_CS_MOVE_START: {
			bool ret = Proc_Packet_MOVE_START(p_session, cs_payload);
			return ret;
		}

		case dfPACKET_CS_MOVE_STOP: {
			auto ret = Proc_Packet_MOVE_STOP(p_session, cs_payload);
			return ret;
		}

		case dfPACKET_CS_ATTACK1: {
			auto ret = Proc_Packet_ATTACK1(p_session, cs_payload);
			return ret;
		}

		case dfPACKET_CS_ATTACK2: {
			auto ret = Proc_Packet_ATTACK2(p_session, cs_payload);
			return ret;
		}


		case dfPACKET_CS_ATTACK3: {
			auto ret = Proc_Packet_ATTACK3(p_session, cs_payload);
			return ret;
		}

		case dfPACKET_CS_ECHO: {
			Proc_Packet_Echo(p_session, cs_payload);
			break;
		}

		default: {
			//OutputDebugString(L"default \n");
			break;
		}
	}
}

bool Proc_Packet_MOVE_START(Session* p_session, ProtocolBuffer* cs_payload) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();

	BYTE action;
	short x, y;

	(*cs_payload) >> action;
	(*cs_payload) >> x;
	(*cs_payload) >> y;

	//--------------------------------
	// ID�� ĳ���� �˻�
	//--------------------------------
	auto iter = character_map.find(p_session->session_id);
	if (iter == character_map.end())
		throw;

	Character* p_character = iter->second;

	//--------------------------------
	// ��ǥ Ʋ���� �Ǵ�, Sync Send
	//--------------------------------
	bool is_sync = false;
	if (abs(p_character->x - x) > SYNC_RANGE || abs(p_character->y - y) > SYNC_RANGE) {
		is_sync = true;

		SectorAround sector_around;
		SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

		sc_packet.Clear();
		moveStart_sync++;
		Log::Warn("[SYNC] Ŭ���̾�Ʈ : Pos(%d, %d), Sector(%d, %d) / ���� : Pos(%d, %d), Sector(%d, %d) ", x, y, x / SECTOR_SIZE, y / SECTOR_SIZE, p_character->x, p_character->y, p_character->cur_sector.x, p_character->cur_sector.y);
		MakePacket_SYNC(&sc_packet, p_character->session_id, p_character->x, p_character->y);
		SendPacket_Around(&sector_around, &sc_packet);
	}

	//--------------------------------
	// ��ǥ �ֽ�ȭ
	//--------------------------------
	p_character->action = (ACTION)action;
	if (!is_sync) {
		p_character->x = x;
		p_character->y = y;
	}

	//--------------------------------
	// ���� �ֽ�ȭ
	//--------------------------------
	switch ((ACTION)action) {
	case ACTION::MOVE_RR:
	case ACTION::MOVE_RU:
	case ACTION::MOVE_RD:
		p_character->dir = DIR::DIR_RR;
		break;

	case ACTION::MOVE_LL:
	case ACTION::MOVE_LU:
	case ACTION::MOVE_LD:
		p_character->dir = DIR::DIR_LL;
		break;
	}

	//------------------------------
	// ���� �ֽ�ȭ �� ������Ʈ ��Ŷ Send
	//------------------------------
	if (SectorFunc::Update_SectorPos(p_character)) {
		SCPacket::CharacterSectorUpdate(p_character);
	}

	//------------------------------
	// Get Sector Around
	//------------------------------
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	//------------------------------
	// Send Move Start
	//------------------------------
	sc_packet.Clear();
	MakePacket_MOVE_START(&sc_packet, p_session->session_id, (ACTION)action, p_character->x, p_character->y);
	SendPacket_Around(&sector_around, &sc_packet);

	pool_protocolBuf.Free(&sc_packet);
	return true;
}

bool Proc_Packet_MOVE_STOP(Session* p_session, ProtocolBuffer* cs_payload) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();

	BYTE dir;
	short x, y;

	(*cs_payload) >> dir;
	(*cs_payload) >> x;
	(*cs_payload) >> y;

	//--------------------------------
	// ID�� ĳ���� �˻�
	//--------------------------------
	auto iter = character_map.find(p_session->session_id);
	if (iter == character_map.end())
		throw;

	Character* p_character = iter->second;

	//--------------------------------
	// ��ǥ Ʋ���� �Ǵ�, Sync Send
	//--------------------------------
	bool is_sync = false;
	if (abs(p_character->x - x) > SYNC_RANGE || abs(p_character->y - y) > SYNC_RANGE) {
		is_sync = true;

		SectorAround sector_around;
		SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

		sc_packet.Clear();
		moveStop_sync++;
		MakePacket_SYNC(&sc_packet, p_character->session_id, p_character->x, p_character->y);
		SendPacket_Around(&sector_around, &sc_packet);
	}

	//--------------------------------
	// ����, ��ǥ �ֽ�ȭ
	//--------------------------------
	p_character->dir = (DIR)dir;
	p_character->action = ACTION::STOP;
	if (!is_sync) {
		p_character->x = x;
		p_character->y = y;
	}

	//------------------------------
	// ���� �ֽ�ȭ �� ������Ʈ ��Ŷ Send
	//------------------------------
	if (SectorFunc::Update_SectorPos(p_character)) {
		SCPacket::CharacterSectorUpdate(p_character);
	}

	//------------------------------
	// Get Sector Around
	//------------------------------
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	//------------------------------
	// Send Move Stop
	//------------------------------
	sc_packet.Clear();
	MakePacket_MOVE_STOP(&sc_packet, p_session->session_id, (DIR)p_character->dir, p_character->x, p_character->y);
	SendPacket_Around(&sector_around, &sc_packet);

	pool_protocolBuf.Free(&sc_packet);
	return true;
}

bool Proc_Packet_ATTACK1(Session* p_session, ProtocolBuffer* cs_payload) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();

	BYTE dir;
	short x, y;

	(*cs_payload) >> dir;
	(*cs_payload) >> x;
	(*cs_payload) >> y;

	const int attack_range_x = ATTACK1_RANGE_X;
	const int attack_range_y = ATTACK1_RANGE_Y;
	const int attack_damage = ATTACK1_DAMAGE;

	//--------------------------------
	// ID�� ĳ���� �˻�
	//--------------------------------
	auto iter = character_map.find(p_session->session_id);
	if (iter == character_map.end())
		throw;

	Character* p_character = iter->second;

	//--------------------------------
	// ��ǥ Ʋ���� �Ǵ�, Sync Send
	//--------------------------------
	bool is_sync = false;
	if (abs(p_character->x - x) > SYNC_RANGE || abs(p_character->y - y) > SYNC_RANGE) {
		is_sync = true;

		SectorAround sector_around;
		SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

		sc_packet.Clear();
		attack_sync++;
		MakePacket_SYNC(&sc_packet, p_character->session_id, p_character->x, p_character->y);
		SendPacket_Around(&sector_around, &sc_packet);
	}

	//--------------------------------
	// ����, ��ǥ �ֽ�ȭ
	//--------------------------------
	p_character->dir = (DIR)dir;
	p_character->action = ACTION::STOP;
	if (!is_sync) {
		p_character->x = x;
		p_character->y = y;
	}

	//------------------------------
	// ���� �ֽ�ȭ �� ������Ʈ ��Ŷ Send
	//------------------------------
	if (SectorFunc::Update_SectorPos(p_character)) {
		SCPacket::CharacterSectorUpdate(p_character);
	}

	//------------------------------
	// Get Sector Around
	//------------------------------
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	//------------------------------
	// Send ATTACK1
	//------------------------------
	sc_packet.Clear();
	MakePacket_ATTACK1(&sc_packet, p_session->session_id, (DIR)p_character->dir, p_character->x, p_character->y);
	SendPacket_Around(&sector_around, &sc_packet);

	//------------------------------
	// Send Damage
	//------------------------------

	// �������� ���� Ž��
	SectorAround damage_sectors;
	damage_sectors.around[0] = p_character->cur_sector;
	damage_sectors.count = 1;

	// ĳ������ ������ RR�̶��
	if (p_character->dir == DIR::DIR_RR) {
		// ���� ���Ͱ� ������ ���� �ƴ϶��,
		if (p_character->cur_sector.x < SECTOR_MAX_X - 1) {
			// ������ ���Ϳ��Ե� ���� �� �ִٸ�,

			if (p_character->cur_sector.x + 1 == (p_character->x + attack_range_x) / SECTOR_SIZE) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y;
				damage_sectors.count++;
			}
		}

		// ĳ���� Y��ǥ ������ ����
		int offset_y = p_character->y % SECTOR_SIZE;
		// �� ���Ϳ� ���� �� ����
		if (offset_y < attack_range_y && p_character->cur_sector.y > 0) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
			damage_sectors.count++;

			// �ִ� 4����
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
				damage_sectors.count++;
			}
		}
		// �Ʒ� ���Ϳ� ���� �� ����
		else if (offset_y > SECTOR_SIZE - attack_range_y && p_character->cur_sector.y < SECTOR_MAX_Y - 1) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
			damage_sectors.count++;

			// �ִ� 4����
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
				damage_sectors.count++;
			}
		}
	}
	// ĳ������ ������ LL�̶��
	else {
		// ���� ���Ͱ� ���ʳ��� �ƴ϶��
		if (p_character->cur_sector.x > 0) {
			// ���� ���Ϳ��Ե� ���� �� �ִٸ�,

			if (p_character->cur_sector.x - 1 == (p_character->x - attack_range_x) / SECTOR_SIZE) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y;
				damage_sectors.count++;
			}
		}

		// ĳ���� Y��ǥ ������ ����
		int offset_y = p_character->y % SECTOR_SIZE;
		// �� ���Ϳ� ���� �� ����
		if (offset_y < attack_range_y && p_character->cur_sector.y > 0) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
			damage_sectors.count++;

			// �ִ� 4���� (������ ���͵� �߰�)
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
				damage_sectors.count++;
			}
		}
		// �Ʒ� ���Ϳ� ���� �� ����
		else if (offset_y > SECTOR_SIZE - attack_range_y && p_character->cur_sector.y < SECTOR_MAX_Y - 1) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
			damage_sectors.count++;

			// �ִ� 4���� (���� �Ʒ� ���͵� �߰�)
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
				damage_sectors.count++;
			}
		}
	}

	// �������� ������ �÷��̾�鿡�� Send
	for (int count = 0; count < damage_sectors.count; count++) {
		for (auto sectors_iter = sectors[damage_sectors.around[count].y][damage_sectors.around[count].x].begin();
			sectors_iter != sectors[damage_sectors.around[count].y][damage_sectors.around[count].x].end();
			sectors_iter++)
		{

			if (*sectors_iter == p_character)
				continue;
			auto p_other_character = *sectors_iter;

			// �ȴ���
			if (p_other_character->y + attack_range_y < p_character->y) continue;
			if (p_other_character->y - attack_range_y > p_character->y) continue;
			if (DIR::DIR_LL == p_character->dir) {
				if (p_character->x < p_other_character->x)
					continue;
				if (p_other_character->x + attack_range_x < p_character->x)
					continue;
			}
			else if (DIR::DIR_RR == p_character->dir) {
				if (p_other_character->x < p_character->x)
					continue;
				if (p_character->x + attack_range_x < p_other_character->x)
					continue;
			}

			// ����
			// ������ ó��
			p_other_character->hp -= attack_damage;

			// ������ ��Ŷ Send
			sc_packet.Clear();
			MakePacket_DAMAGE(&sc_packet, p_session->session_id, p_other_character->session_id, p_other_character->hp);
			SectorAround otherCharacter_sector_around;
			SectorFunc::Get_SectorAround(p_other_character->cur_sector.x, p_other_character->cur_sector.y, &otherCharacter_sector_around);
			SendPacket_Around(&otherCharacter_sector_around, &sc_packet);
			goto DAMAGE_SEND_COMPLETE;
		}
	}
DAMAGE_SEND_COMPLETE:

	pool_protocolBuf.Free(&sc_packet);
	return true;
}

bool Proc_Packet_ATTACK2(Session* p_session, ProtocolBuffer* cs_payload) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();

	BYTE dir;
	short x, y;

	(*cs_payload) >> dir;
	(*cs_payload) >> x;
	(*cs_payload) >> y;

	const int attack_range_x = ATTACK2_RANGE_X;
	const int attack_range_y = ATTACK2_RANGE_Y;
	const int attack_damage = ATTACK2_DAMAGE;

	//--------------------------------
	// ID�� ĳ���� �˻�
	//--------------------------------
	auto iter = character_map.find(p_session->session_id);
	if (iter == character_map.end())
		throw;

	Character* p_character = iter->second;

	//--------------------------------
	// ��ǥ Ʋ���� �Ǵ�, Sync Send
	//--------------------------------
	bool is_sync = false;
	if (abs(p_character->x - x) > SYNC_RANGE || abs(p_character->y - y) > SYNC_RANGE) {
		is_sync = true;

		SectorAround sector_around;
		SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

		sc_packet.Clear();
		attack_sync++;
		MakePacket_SYNC(&sc_packet, p_character->session_id, p_character->x, p_character->y);
		SendPacket_Around(&sector_around, &sc_packet);
	}

	//--------------------------------
	// ����, ��ǥ �ֽ�ȭ
	//--------------------------------
	p_character->dir = (DIR)dir;
	p_character->action = ACTION::STOP;
	if (!is_sync) {
		p_character->x = x;
		p_character->y = y;
	}

	//------------------------------
	// ���� �ֽ�ȭ �� ������Ʈ ��Ŷ Send
	//------------------------------
	if (SectorFunc::Update_SectorPos(p_character)) {
		SCPacket::CharacterSectorUpdate(p_character);	
	}

	//------------------------------
	// Get Sector Around
	//------------------------------
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	//------------------------------
	// Send ATTACK1
	//------------------------------
	sc_packet.Clear();
	MakePacket_ATTACK2(&sc_packet, p_session->session_id, (DIR)p_character->dir, p_character->x, p_character->y);
	SendPacket_Around(&sector_around, &sc_packet);

	//------------------------------
	// Send Damage
	//------------------------------

	// �������� ���� Ž��
	SectorAround damage_sectors;
	damage_sectors.around[0] = p_character->cur_sector;
	damage_sectors.count = 1;

	// ĳ������ ������ RR�̶��
	if (p_character->dir == DIR::DIR_RR) {
		// ���� ���Ͱ� ������ ���� �ƴ϶��,
		if (p_character->cur_sector.x < SECTOR_MAX_X - 1) {
			// ������ ���Ϳ��Ե� ���� �� �ִٸ�,

			if (p_character->cur_sector.x + 1 == (p_character->x + attack_range_x) / SECTOR_SIZE) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y;
				damage_sectors.count++;
			}
		}

		// ĳ���� Y��ǥ ������ ����
		int offset_y = p_character->y % SECTOR_SIZE;
		// �� ���Ϳ� ���� �� ����
		if (offset_y < attack_range_y && p_character->cur_sector.y > 0) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
			damage_sectors.count++;

			// �ִ� 4����
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
				damage_sectors.count++;
			}
		}
		// �Ʒ� ���Ϳ� ���� �� ����
		else if (offset_y > SECTOR_SIZE - attack_range_y && p_character->cur_sector.y < SECTOR_MAX_Y - 1) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
			damage_sectors.count++;

			// �ִ� 4����
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
				damage_sectors.count++;
			}
		}
	}
	// ĳ������ ������ LL�̶��
	else {
		// ���� ���Ͱ� ���ʳ��� �ƴ϶��
		if (p_character->cur_sector.x > 0) {
			// ���� ���Ϳ��Ե� ���� �� �ִٸ�,

			if (p_character->cur_sector.x - 1 == (p_character->x - attack_range_x) / SECTOR_SIZE) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y;
				damage_sectors.count++;
			}
		}

		// ĳ���� Y��ǥ ������ ����
		int offset_y = p_character->y % SECTOR_SIZE;
		// �� ���Ϳ� ���� �� ����
		if (offset_y < attack_range_y && p_character->cur_sector.y > 0) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
			damage_sectors.count++;

			// �ִ� 4���� (������ ���͵� �߰�)
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
				damage_sectors.count++;
			}
		}
		// �Ʒ� ���Ϳ� ���� �� ����
		else if (offset_y > SECTOR_SIZE - attack_range_y && p_character->cur_sector.y < SECTOR_MAX_Y - 1) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
			damage_sectors.count++;

			// �ִ� 4���� (���� �Ʒ� ���͵� �߰�)
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
				damage_sectors.count++;
			}
		}
	}

	// �������� ������ �÷��̾�鿡�� Send
	for (int count = 0; count < damage_sectors.count; count++) {
		for (auto sectors_iter = sectors[damage_sectors.around[count].y][damage_sectors.around[count].x].begin();
			sectors_iter != sectors[damage_sectors.around[count].y][damage_sectors.around[count].x].end();
			sectors_iter++)
		{

			if (*sectors_iter == p_character)
				continue;
			auto p_other_character = *sectors_iter;

			// �ȴ���
			if (p_other_character->y + attack_range_y < p_character->y) continue;
			if (p_other_character->y - attack_range_y > p_character->y) continue;
			if (DIR::DIR_LL == p_character->dir) {
				if (p_character->x < p_other_character->x)
					continue;
				if (p_other_character->x + attack_range_x < p_character->x)
					continue;
			}
			else if (DIR::DIR_RR == p_character->dir) {
				if (p_other_character->x < p_character->x)
					continue;
				if (p_character->x + attack_range_x < p_other_character->x)
					continue;
			}

			// ����
			// ������ ó��
			p_other_character->hp -= attack_damage;

			// ������ ��Ŷ Send
			sc_packet.Clear();
			MakePacket_DAMAGE(&sc_packet, p_session->session_id, p_other_character->session_id, p_other_character->hp);
			SectorAround otherCharacter_sector_around;
			SectorFunc::Get_SectorAround(p_other_character->cur_sector.x, p_other_character->cur_sector.y, &otherCharacter_sector_around);
			SendPacket_Around(&otherCharacter_sector_around, &sc_packet);
			goto DAMAGE_SEND_COMPLETE;
		}
	}
DAMAGE_SEND_COMPLETE:

	pool_protocolBuf.Free(&sc_packet);
	return true;
}

bool Proc_Packet_ATTACK3(Session* p_session, ProtocolBuffer* cs_payload) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();

	BYTE dir;
	short x, y;

	(*cs_payload) >> dir;
	(*cs_payload) >> x;
	(*cs_payload) >> y;

	const int attack_range_x = ATTACK3_RANGE_X;
	const int attack_range_y = ATTACK3_RANGE_Y;
	const int attack_damage = ATTACK3_DAMAGE;


	//--------------------------------
	// ID�� ĳ���� �˻�
	//--------------------------------
	auto iter = character_map.find(p_session->session_id);
	if (iter == character_map.end())
		throw;

	Character* p_character = iter->second;

	//--------------------------------
	// ��ǥ Ʋ���� �Ǵ�, Sync Send
	//--------------------------------
	bool is_sync = false;
	if (abs(p_character->x - x) > SYNC_RANGE || abs(p_character->y - y) > SYNC_RANGE) {
		is_sync = true;

		SectorAround sector_around;
		SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

		sc_packet.Clear();
		attack_sync++;
		MakePacket_SYNC(&sc_packet, p_character->session_id, p_character->x, p_character->y);
		SendPacket_Around(&sector_around, &sc_packet);
	}

	//--------------------------------
	// ����, ��ǥ �ֽ�ȭ
	//--------------------------------
	p_character->dir = (DIR)dir;
	p_character->action = ACTION::STOP;
	if (!is_sync) {
		p_character->x = x;
		p_character->y = y;
	}

	//------------------------------
	// ���� �ֽ�ȭ �� ������Ʈ ��Ŷ Send
	//------------------------------
	if (SectorFunc::Update_SectorPos(p_character)) {
		SCPacket::CharacterSectorUpdate(p_character);
	}

	//------------------------------
	// Get Sector Around
	//------------------------------
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	//------------------------------
	// Send ATTACK1
	//------------------------------
	sc_packet.Clear();
	MakePacket_ATTACK3(&sc_packet, p_session->session_id, (DIR)p_character->dir, p_character->x, p_character->y);
	SendPacket_Around(&sector_around, &sc_packet);

	//------------------------------
	// Send Damage
	//------------------------------

	// �������� ���� Ž��
	SectorAround damage_sectors;
	damage_sectors.around[0] = p_character->cur_sector;
	damage_sectors.count = 1;

	// ĳ������ ������ RR�̶��
	if (p_character->dir == DIR::DIR_RR) {
		// ���� ���Ͱ� ������ ���� �ƴ϶��,
		if (p_character->cur_sector.x < SECTOR_MAX_X - 1) {
			// ������ ���Ϳ��Ե� ���� �� �ִٸ�,

			if (p_character->cur_sector.x + 1 == (p_character->x + attack_range_x) / SECTOR_SIZE) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y;
				damage_sectors.count++;
			}
		}

		// ĳ���� Y��ǥ ������ ����
		int offset_y = p_character->y % SECTOR_SIZE;
		// �� ���Ϳ� ���� �� ����
		if (offset_y < attack_range_y && p_character->cur_sector.y > 0) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
			damage_sectors.count++;

			// �ִ� 4����
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
				damage_sectors.count++;
			}
		}
		// �Ʒ� ���Ϳ� ���� �� ����
		else if (offset_y > SECTOR_SIZE - attack_range_y && p_character->cur_sector.y < SECTOR_MAX_Y - 1) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
			damage_sectors.count++;

			// �ִ� 4����
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x + 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
				damage_sectors.count++;
			}
		}
	}
	// ĳ������ ������ LL�̶��
	else {
		// ���� ���Ͱ� ���ʳ��� �ƴ϶��
		if (p_character->cur_sector.x > 0) {
			// ���� ���Ϳ��Ե� ���� �� �ִٸ�,

			if (p_character->cur_sector.x - 1 == (p_character->x - attack_range_x) / SECTOR_SIZE) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y;
				damage_sectors.count++;
			}
		}

		// ĳ���� Y��ǥ ������ ����
		int offset_y = p_character->y % SECTOR_SIZE;
		// �� ���Ϳ� ���� �� ����
		if (offset_y < attack_range_y && p_character->cur_sector.y > 0) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
			damage_sectors.count++;

			// �ִ� 4���� (������ ���͵� �߰�)
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y - 1;
				damage_sectors.count++;
			}
		}
		// �Ʒ� ���Ϳ� ���� �� ����
		else if (offset_y > SECTOR_SIZE - attack_range_y && p_character->cur_sector.y < SECTOR_MAX_Y - 1) {
			damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x;
			damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
			damage_sectors.count++;

			// �ִ� 4���� (���� �Ʒ� ���͵� �߰�)
			if (damage_sectors.count == 3) {
				damage_sectors.around[damage_sectors.count].x = p_character->cur_sector.x - 1;
				damage_sectors.around[damage_sectors.count].y = p_character->cur_sector.y + 1;
				damage_sectors.count++;
			}
		}
	}

	// �������� ������ �÷��̾�鿡�� Send
	for (int count = 0; count < damage_sectors.count; count++) {
		for (auto sectors_iter = sectors[damage_sectors.around[count].y][damage_sectors.around[count].x].begin();
			sectors_iter != sectors[damage_sectors.around[count].y][damage_sectors.around[count].x].end();
			sectors_iter++)
		{

			if (*sectors_iter == p_character)
				continue;
			auto p_other_character = *sectors_iter;

			// �ȴ���
			if (p_other_character->y + attack_range_y < p_character->y) continue;
			if (p_other_character->y - attack_range_y > p_character->y) continue;
			if (DIR::DIR_LL == p_character->dir) {
				if (p_character->x < p_other_character->x)
					continue;
				if (p_other_character->x + attack_range_x < p_character->x)
					continue;
			}
			else if (DIR::DIR_RR == p_character->dir) {
				if (p_other_character->x < p_character->x)
					continue;
				if (p_character->x + attack_range_x < p_other_character->x)
					continue;
			}

			// ����
			// ������ ó��
			p_other_character->hp -= attack_damage;

			// ������ ��Ŷ Send
			sc_packet.Clear();
			MakePacket_DAMAGE(&sc_packet, p_session->session_id, p_other_character->session_id, p_other_character->hp);
			SectorAround otherCharacter_sector_around;
			SectorFunc::Get_SectorAround(p_other_character->cur_sector.x, p_other_character->cur_sector.y, &otherCharacter_sector_around);
			SendPacket_Around(&otherCharacter_sector_around, &sc_packet);
			goto DAMAGE_SEND_COMPLETE;
		}
	}
DAMAGE_SEND_COMPLETE:

	pool_protocolBuf.Free(&sc_packet);
	return true;
}

bool  Proc_Packet_Echo(Session* p_session, ProtocolBuffer* cs_payload) {
	return true;
}

// * ĳ���� ���� ���� �� ȣ�� (�Ǿ����)
// 1. Remove Sector�� (�̵�)ĳ���� ���� ��Ŷ Send
// 2. (�̵�)ĳ���Ϳ��� Remove Sector ĳ���� ���� ��Ŷ
// 3. Add Sector�� (�̵�)ĳ���� ����(+�̵�) ��Ŷ
// 4. (����)ĳ���Ϳ��� Add Sector ����(+�̵�) ��Ŷ
void SCPacket::CharacterSectorUpdate(Character* p_character) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();

	SectorAround remove_sectorAround, add_sectorAround;
	SectorFunc::Get_ChangedSectorAround(p_character, &remove_sectorAround, &add_sectorAround); // FIX

	//------------------------------
	// 1. Remove Sector�� (����)ĳ���� ���� ��Ŷ Send
	//------------------------------
	sc_packet.Clear();
	MakePacket_DELETE_CHARACTER(&sc_packet, p_character->session_id);
	for (int i = 0; i < remove_sectorAround.count; i++) {
		SendPacket_SectorOne(remove_sectorAround.around[i].x, remove_sectorAround.around[i].y, &sc_packet, nullptr);
	}

	//------------------------------
	// 2. (����)ĳ���Ϳ��� Remove Sector ���� ��Ŷ Send
	//------------------------------

	for (int i = 0; i < remove_sectorAround.count; i++) {
		const auto& sector = sectors[remove_sectorAround.around[i].y][remove_sectorAround.around[i].x];
		for (auto iter = sector.begin(); iter != sector.end(); iter++) {
			const auto& p_other_character = *iter;

			sc_packet.Clear();
			MakePacket_DELETE_CHARACTER(&sc_packet, p_other_character->session_id);
			SendPacket_Unicast(p_character->p_session, &sc_packet);
		}
	}

	//------------------------------
	// 3. Add Sector�� (����)ĳ���� ����(+�̵�) ��Ŷ Send
	//------------------------------
	sc_packet.Clear();
	MakePacket_CREATE_OTHER_CHARACTER(&sc_packet, p_character->session_id, p_character->dir, p_character->x, p_character->y, p_character->hp);
	if (p_character->action < ACTION::STOP) {
		MakePacket_MOVE_START(&sc_packet, p_character->session_id, p_character->action, p_character->x, p_character->y);
	}

	for (int i = 0; i < add_sectorAround.count; i++) {
		SendPacket_SectorOne(add_sectorAround.around[i].x, add_sectorAround.around[i].y, &sc_packet, nullptr);
	}

	//------------------------------
	// 4. (����)ĳ���Ϳ��� Add Sector ����(+�̵�) ��Ŷ Send
	//------------------------------

	for (int i = 0; i < add_sectorAround.count; i++) {
		const auto& sector = sectors[add_sectorAround.around[i].y][add_sectorAround.around[i].x];
		for (auto iter = sector.begin(); iter != sector.end(); iter++) {
			const auto& p_other_character = *iter;

			sc_packet.Clear();
			MakePacket_CREATE_OTHER_CHARACTER(&sc_packet, p_other_character->session_id, p_other_character->dir, p_other_character->x, p_other_character->y, p_other_character->hp);
			if (p_other_character->action < ACTION::STOP)
				MakePacket_MOVE_START(&sc_packet, p_other_character->session_id, p_other_character->action, p_other_character->x, p_other_character->y);
			SendPacket_Unicast(p_character->p_session, &sc_packet);
		}
	}

	pool_protocolBuf.Free(&sc_packet);
}

void SCPacket::Disconnect_Character(Character* p_character) {
	ProtocolBuffer& sc_packet = *pool_protocolBuf.Alloc();
	SectorAround sector_around;
	SectorFunc::Get_SectorAround(p_character->cur_sector.x, p_character->cur_sector.y, &sector_around);

	//------------------------------
	// ĳ���� ���� ��Ŷ Send
	//------------------------------
	sc_packet.Clear();
	MakePacket_DELETE_CHARACTER(&sc_packet, p_character->session_id);
	SendPacket_Around(&sector_around, &sc_packet, p_character->p_session);

	pool_protocolBuf.Free(&sc_packet);
}

int logic_timer = 0;
DWORD prev_logic_time = 0;
DWORD cur_logic_time = 0;
extern unsigned logic_count;

void Update(DWORD delta_time) {
	// logic timer�� ����ġ�� 40ms�� �ɶ����� 
	logic_timer += delta_time;
	if (logic_timer < FRAME_MS) 
		return;

	logic_timer -= FRAME_MS;
	logic_count++;

	auto logic_cur_time = timeGetTime();
	for (auto iter = character_map.begin(); iter != character_map.end(); iter++) {
		auto p_character = iter->second;

		// ĳ���� ���
		if (p_character->hp <= 0) {
			Input_DisconnectQ(p_character->p_session, "p_character->hp <= 0");
			continue;
		}

		// Ÿ�Ӿƿ�
		if (logic_cur_time - p_character->p_session->last_recvTime > dfNETWORK_PACKET_RECV_TIMEOUT) {
			Input_DisconnectQ(p_character->p_session, "time out");
			continue;
		}

		//------------------------------
		// ĳ���� �̵� (�̵����� ĳ���� x, y ���(�ֽ�ȭ))
		//------------------------------
		bool is_move = false;
		switch (p_character->action) {
			case ACTION::MOVE_UU: {
				// MOVE_UU
				if (p_character->y <= SPEED_PLAYER_Y) {
					p_character->y = RANGE_MOVE_TOP;
				}
				else {
					p_character->y -= SPEED_PLAYER_Y;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_UU ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_DD: {
				// MOVE_DD
				if (p_character->y >= RANGE_MOVE_BOTTOM - SPEED_PLAYER_Y) {
					p_character->y = RANGE_MOVE_BOTTOM - 1;
				}
				else {
					p_character->y += SPEED_PLAYER_Y;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_DD ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_LL: {
				// MOVE_LL
				if (p_character->x <= SPEED_PLAYER_X) {
					p_character->x = RANGE_MOVE_LEFT;
				}
				else {
					p_character->x -= SPEED_PLAYER_X;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_LL ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_RR: {
				// MOVE_RR
				if (p_character->x >= RANGE_MOVE_RIGHT - SPEED_PLAYER_X) {
					p_character->x = RANGE_MOVE_RIGHT - 1;
				}
				else {
					p_character->x += SPEED_PLAYER_X;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_RR ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_LU: {
				// �� üũ
				if (p_character->x <= RANGE_MOVE_LEFT)	break;
				if (p_character->y <= RANGE_MOVE_TOP)	break;

				// MOVE_LL
				if (p_character->x <= SPEED_PLAYER_X) {
					p_character->x = RANGE_MOVE_LEFT;
				}
				else {
					p_character->x -= SPEED_PLAYER_X;
					is_move = true;
				}

				// MOVE_UU
				if (p_character->y <= SPEED_PLAYER_Y) {
					p_character->y = RANGE_MOVE_TOP;
				}
				else {
					p_character->y -= SPEED_PLAYER_Y;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_LU ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_LD: {
				// �� üũ
				if (p_character->x <= RANGE_MOVE_LEFT)		break;
				if (p_character->y >= RANGE_MOVE_BOTTOM - 1)	break;

				// MOVE_LL
				if (p_character->x <= SPEED_PLAYER_X) {
					p_character->x = RANGE_MOVE_LEFT;
				}
				else {
					p_character->x -= SPEED_PLAYER_X;
					is_move = true;
				}

				// MOVE_DD
				if (p_character->y >= RANGE_MOVE_BOTTOM - SPEED_PLAYER_Y) {
					p_character->y = RANGE_MOVE_BOTTOM - 1;
				}
				else {
					p_character->y += SPEED_PLAYER_Y;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_LD ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_RU: {
				// �� üũ
				if (p_character->x >= RANGE_MOVE_RIGHT - 1)	break;
				if (p_character->y <= RANGE_MOVE_TOP)	break;

				// MOVE_RR
				if (p_character->x >= RANGE_MOVE_RIGHT - SPEED_PLAYER_X) {
					p_character->x = RANGE_MOVE_RIGHT - 1;
				}
				else {
					p_character->x += SPEED_PLAYER_X;
					is_move = true;
				}

				// MOVE_UU
				if (p_character->y <= SPEED_PLAYER_Y) {
					p_character->y = RANGE_MOVE_TOP;
				}
				else {
					p_character->y -= SPEED_PLAYER_Y;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_RU ", p_character->session_id);
				break;
			}

			case ACTION::MOVE_RD: {
				// �� üũ
				if (p_character->x >= RANGE_MOVE_RIGHT - 1)		break;
				if (p_character->y >= RANGE_MOVE_BOTTOM - 1)	break;

				// MOVE_RR
				if (p_character->x >= RANGE_MOVE_RIGHT - SPEED_PLAYER_X) {
					p_character->x = RANGE_MOVE_RIGHT - 1;
				}
				else {
					p_character->x += SPEED_PLAYER_X;
					is_move = true;
				}

				// MOVE_DD
				if (p_character->y >= RANGE_MOVE_BOTTOM - SPEED_PLAYER_Y) {
					p_character->y = RANGE_MOVE_BOTTOM - 1;
				}
				else {
					p_character->y += SPEED_PLAYER_Y;
					is_move = true;
				}
				//printf("Sessoin id : %d, MOVE_RD ", p_character->session_id);
				break;
			}

			default: {
				break;
			}
		}

		//------------------------------
		// ���� ������Ʈ, ���� ������Ʈ ��Ŷ Send
		//------------------------------
		if (is_move) {
			if (SectorFunc::Update_SectorPos(p_character))
				SCPacket::CharacterSectorUpdate(p_character);
		}
	}
}

int Complete_RecvPacket(Session* p_session) {
	PACKET_HEADER packet_header;
	int iRecvQSize;

	iRecvQSize = p_session->recv_buf.Get_UseSize();

	// ��Ŷ��� �̻����� �˻�
	if (sizeof(packet_header) > iRecvQSize)
		return 1;

	// PACKET CODE �˻�
	p_session->recv_buf.Peek((char*)&packet_header, sizeof(packet_header));
	if (PACKET_CODE != packet_header.byCode)
		return -1;

	if (iRecvQSize < packet_header.bySize + sizeof(packet_header))
		return 1;

	p_session->recv_buf.Move_Front(sizeof(packet_header));

	ProtocolBuffer& cs_payload = *pool_protocolBuf.Alloc();
	cs_payload.Clear();

	// Payload �� ���۷� ����
	auto dequeue_size = p_session->recv_buf.Dequeue(cs_payload.Get_writePos(), packet_header.bySize);
	if (dequeue_size != packet_header.bySize) {
		throw;
	}
	cs_payload.Move_Wp(dequeue_size);

	// ��Ŷó�� �Լ� call
	auto ret = Proc_Packet((Session*)p_session, (unsigned char)packet_header.byType, (ProtocolBuffer*)&cs_payload);
	pool_protocolBuf.Free(&cs_payload);

	if (!ret)
		return -1;
	return 0;
}