#include "stdafx.h"
#include "Server.h"
#include "Error_Check.h"
#include "Actor.h"
#include "Protocol.h"

using namespace std;

StarServer::StarServer() {}

StarServer& StarServer::Get_Inst() {
	static StarServer* Inst = new StarServer;
	return *Inst;
}

bool StarServer::SendUnicast(void* p_packet, Player* p_player) {
	auto ret_enqueue_size = p_player->send_buf.Enqueue((const char*)p_packet, PACKET_SIZE);
	Error_Check(PACKET_SIZE != ret_enqueue_size);
	return true;

	//auto error_num = Error_Check(SOCKET_ERROR == send(p_player->player_socket, (const char*)p_packet, PACKET_SIZE, NULL));
	//if (error_num == WSAECONNRESET) {
	//	p_player->flag_disconnect = true;
	//	return false;
	//}
	//return true;
}
 
bool StarServer::SendBroadCast(void* p_packet, const char* exclude) {
	for (list<Player>::iterator iter = players.begin(); iter != players.end(); iter++) {
		// 제외대상이 있다면 제외대상 체크
		if (exclude != nullptr) {
			for (; *exclude != INVALID_ID; exclude++) {
				// 해당 플레이어가 브로드캐스트 제외대상 이라면 다음 플레이어로 넘어감
				if (*exclude == iter->id)
					goto next;
			}
		}
		SendUnicast(p_packet, &(*iter));

	next:
		continue;
	}
	return true;
}

bool StarServer::AcceptProc()
{
	if (players.size() >= MAX_CLIENT)
		return false;

	// 플레이어 생성
	players.emplace_back();
	auto& new_player = *(--players.end());

	// accept
	SOCKADDR_IN host_addr;
	ZeroMemory(&host_addr, sizeof(host_addr));
	int addr_size = sizeof(host_addr);

	new_player.player_socket = accept(listen_socket, (SOCKADDR*)&host_addr, &addr_size);
	Error_Check(INVALID_SOCKET == new_player.player_socket);

	// new_player 데이터 셋팅
	new_player.id = Get_id();
	new_player.ip = ntohl(host_addr.sin_addr.s_addr);
	new_player.port = ntohs(host_addr.sin_port);

	// 디버그 코드
	printf("id : %d, Accept \n", new_player.id);

	// Send_unicast->new_player, get_id 패킷
	packet_get_id* get_id_packet = new packet_get_id;
	get_id_packet->id = new_player.id;
	get_id_packet->type = PACKET_GET_ID;
	get_id_packet->zero = 0;
	SendUnicast(get_id_packet, &new_player);
	delete get_id_packet;

	// Send_broadcast, new_player create 패킷
	packet_create_character* create_packet = new packet_create_character;
	create_packet->id = new_player.id;
	create_packet->type = PACKET_CREATE_CHARACTER;
	create_packet->x = new_player.x;
	create_packet->y = new_player.y;
	SendBroadCast(create_packet);
	delete create_packet;

	for (list<Player>::iterator iter = players.begin(); iter != players.end(); iter++) {
		if (iter->id == new_player.id)
			continue;
		auto& old_player = *iter;

		// old_player crate 패킷 unicast -> new_player
		packet_create_character* packet = new packet_create_character;
		packet->id = old_player.id;
		packet->type = PACKET_CREATE_CHARACTER;
		packet->x = old_player.x;
		packet->y = old_player.y;
		SendUnicast(packet, &new_player);
		delete packet;
	}

	return true;
}

bool StarServer::RecvProc(Player* p_player) {
	auto recv_size = recv(p_player->player_socket, recv_buf, MAX_BUF, NULL);
	auto error_num = Error_Check(SOCKET_ERROR == recv_size);
	// RST 예외처리
	if (error_num == WSAECONNRESET) {
		p_player->flag_disconnect = true;
		return false;
	}

	p_player->recv_buf.Enqueue(recv_buf, recv_size);

	while (1) {
		if (p_player->recv_buf.GetUseSize() < 16) 
			break;
		
		// 패킷 컨텐츠 처리 로직
		int packet_type = 0;
		p_player->recv_buf.Peek((char*)&packet_type, 4);
		cout << "packet type : " << packet_type << endl;

		switch (packet_type)
		{
		case PACKET_MOVE: {
			p_player->x = ((packet_move*)recv_buf)->x;
			p_player->y = ((packet_move*)recv_buf)->y;

			packet_move packet;
			p_player->recv_buf.Peek((char*)&packet, sizeof(packet));

			char exclue[2] = { packet.id, INVALID_ID };
			SendBroadCast(&packet, exclue);
			p_player->recv_buf.MoveFront(sizeof(packet));
			break;
		}

		default:
			printf("\n알 수 없는 패킷 타입 : %d \n", packet_type);
			CRASH();
			break;
		}
	}

	return true;
}

// Recv, Send 에러 시 호출?
void StarServer::Disconnect()
{
	char remove_id_array[MAX_CLIENT];
	memset(remove_id_array, INVALID_ID, MAX_CLIENT);
	short remove_num = 0;

	for (list<Player>::iterator iter = players.begin(); iter != players.end();) {
		if (iter->flag_disconnect) {
			remove_id_array[remove_num++] = iter->id;
			iter->Invalidate();
			players.erase(iter++);
		}
		else
			iter++;
	}

	//123(45)
	for (int i = 0; i < remove_num; i++) {
		packet_remove_character* remove_packet = new packet_remove_character;
		remove_packet->id = remove_id_array[i];
		remove_packet->type = PACKET_REMOVE_CHARACTER;
		remove_packet->zero = 0;

		SendBroadCast(remove_packet);
		delete remove_packet;
	}
}

int StarServer::Get_id()
{
	static int id = -1;
	return ++id;
}

bool StarServer::Init()
{
	WSADATA wsaData;
	Error_Check(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0);

	// set IP, PORT
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	// set listen_socket
	listen_socket = socket(PF_INET, SOCK_STREAM, 0);
	Error_Check(SOCKET_ERROR == listen_socket);

	// do listen, bind 
	Error_Check(SOCKET_ERROR == bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)));
	Error_Check(SOCKET_ERROR == listen(listen_socket, SOMAXCONN));

	return true;
}

void StarServer::Run() {
	cout << "Run()" << endl;

	fd_set recv_set;
	fd_set send_set;

	while (true) {
		FD_ZERO(&recv_set);
		FD_ZERO(&send_set);

		FD_SET(listen_socket, &recv_set);
		for (list<Player>::iterator iter = players.begin(); iter != players.end(); iter++) {
			if (iter->player_socket != INVALID_SOCKET) {
				FD_SET(iter->player_socket, &recv_set);
				printf("id : %d, recv set \n", iter->id);

				if (!iter->send_buf.Empty()) {
					printf("id : %d,  send set \n", iter->id);
					FD_SET(iter->player_socket, &send_set);
				}
			}
		}

		auto ret_select = select(NULL, &recv_set, &send_set, NULL, NULL);
		cout << "Event Select" << endl;
		Error_Check(SOCKET_ERROR == ret_select);

		if (FD_ISSET(listen_socket, &recv_set) != 0) {
			AcceptProc();
		}
		else {
			for (list<Player>::iterator iter = players.begin(); iter != players.end(); iter++) {
				if (FD_ISSET(iter->player_socket, &recv_set) != 0) {
					//cout << "player : " << (int)iter->id << "에게로부터 recv" << endl;
					RecvProc(&(*iter));
				}
				if (FD_ISSET(iter->player_socket, &send_set) != 0) {
					// send 버퍼 데이터 전부 복사
					char tmp_buf[256];
					auto peek_size = iter->send_buf.Peek(tmp_buf, 256);
					// 일괄 send
					auto send_size = send(iter->player_socket, tmp_buf, peek_size, NULL);
					iter->send_buf.MoveFront(send_size);
				}
			}
		}

		Disconnect();

		// 디버그 코드
		Render();
	}
}


void StarServer::Close() {
	WSACleanup();
}