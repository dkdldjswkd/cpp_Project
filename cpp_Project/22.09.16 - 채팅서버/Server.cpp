#include "stdafx.h"
#include "Server.h"
#include "PACKET.h"
#include "Session_NickName.h"
#include "ChatRoom.h"

using namespace std;

// NETWORK
SOCKET listen_sock;
SOCKADDR_IN server_addr; 
SOCKADDR_IN client_addr;

// Session
unordered_map<session_id, Session*> session_map;
set<NickName*, NickName_Compare> nickName_table;
queue<session_id> disconnect_queue;

// Content
unordered_map<room_no, ChatRoom*> chatRoom_map;
unordered_set<wstring> roomName_table;

// POOL
ObjectPool<ProtocolBuffer> protocolBuffer_pool;
ObjectPool<NickName> nickName_pool;
ObjectPool<Session> session_pool;
ObjectPool<ChatRoom> chatRoom_pool;

Session* Find_Session(session_id id) {
	auto iter = session_map.find(id);
	if (iter == session_map.end())
		return nullptr;

	return iter->second;
}

// �ش� ������ ���� �� ��ȯ
room_no Find_SessionRoom(session_id id) {
	auto p_session = Find_Session(id);
	if (p_session == nullptr)
		return 0;

	return p_session->no;
}

// ���� ���� �� �濡 �����ϴ� ���� �� ��ȯ
int Exit_room(session_id id) {
	auto p_session = Find_Session(id);

	// chatRoom_map.find �� end�� �ȵ�
	auto p_chatRoom = chatRoom_map.find(p_session->no)->second;

	auto& room_session_list = p_chatRoom->session_list;

	// �濡�� �ش� ���� ����
	for (auto iter = room_session_list.begin(); iter != room_session_list.end(); iter++) {
		if (*iter == id) {
			room_session_list.erase(iter);
		}
	}

	// ���� �������� �� �ʱ�ȭ
	p_session->no = 0;

	return p_chatRoom->session_list.size();
}

list<session_id>* Get_room_session_list(room_no no) {
	auto chatRoom_map_iter = chatRoom_map.find(no);

	if (chatRoom_map_iter == chatRoom_map.end())
		return nullptr;

	return &chatRoom_map_iter->second->session_list;
}

bool emplace_disconnect_queue(session_id id) {
	Session* p_session = Find_Session(id);
	if (p_session->disconnect_flag == true)
		return false;

	p_session->disconnect_flag = true;
	disconnect_queue.emplace(id);
	return true;
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

	// listen ()
	if (SOCKET_ERROR == listen(listen_sock, SOMAXCONN)) {
		auto erro_num = WSAGetLastError();
		CRASH();
	}

	return true;
}

void NetWork_Process(){
	int		sessionTable_id		[FD_SETSIZE];
	SOCKET	sessoinTable_socket	[FD_SETSIZE];

	FD_SET read_set;
	FD_SET write_set;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	memset(sessionTable_id, -1, sizeof(int) * FD_SETSIZE);
	memset(sessoinTable_socket, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
	int socket_count = 0;

	// �������� ���
	FD_SET(listen_sock, &read_set);
	sessionTable_id[socket_count] = 0;
	sessoinTable_socket[socket_count] = listen_sock;
	socket_count++;

	// ��� ���ǿ� ���Ͽ� ���� ���� SET�� ���
	for (auto iter = session_map.begin(); iter != session_map.end(); iter++) {
		Session* p_session = iter->second;

		sessionTable_id[socket_count] = p_session->id;
		sessoinTable_socket[socket_count] = p_session->sock;

		FD_SET(p_session->sock, &read_set);
		if(!p_session->send_buf.Empty())
			FD_SET(p_session->sock, &write_set);

		socket_count++;

		// 64���� ����(����)�� ���Ͽ� Select
		if (FD_SETSIZE <= socket_count) {
			Select_Proc(sessionTable_id, sessoinTable_socket, &read_set, &write_set);

			FD_ZERO(&read_set);
			FD_ZERO(&write_set);
			memset(sessionTable_id, -1, sizeof(int) * FD_SETSIZE);
			memset(sessoinTable_socket, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			socket_count = 0;
		}
	}

	// 64�� �̸� ����(����)�� ���Ͽ� Select
	if (0 < socket_count) {
		Select_Proc(sessionTable_id, sessoinTable_socket, &read_set, &write_set);
	}

	// Disconnect
	for (int i = 0; i < disconnect_queue.size(); i++) {
		session_id id = disconnect_queue.front();
		disconnect_queue.pop();

		auto p_session = Find_Session(id);
		p_session->Disconnect();
		session_pool.Free(p_session);
		session_map.erase(id);
	}
}

void Select_Proc(int* p_table_id, SOCKET* p_table_socket, FD_SET* p_readSet, FD_SET* p_writeSet) {
	timeval t{ 0,0 };

	auto ret_select = select(NULL, p_readSet, p_writeSet, NULL, &t);
	if (0 < ret_select) {
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (p_table_socket[i] == INVALID_SOCKET)
				continue;

			if (FD_ISSET(p_table_socket[i], p_writeSet)) {
				Send_Proc(p_table_id[i]);
			}
			if (FD_ISSET(p_table_socket[i], p_readSet)) {
				if (p_table_id[i] == 0) 
					Accept_Proc();
				else
					Recv_Proc(p_table_id[i]);
			}
		}
	}
	else if (ret_select == INVALID_SOCKET) {
		wprintf(L"select socket error \n");
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

int client_addr_size = sizeof(client_addr);

void Accept_Proc() {
	// Accept & Session_Info Set
	SOCKET accept_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_size);
	if (accept_sock == INVALID_SOCKET) {
		switch (WSAGetLastError()) {
			case 10054: {
				break;
			}

			default:{
				CRASH();
				break;
			}
		}
	}

	session_id id = Get_Session_ID();
	auto p_session = session_pool.Alloc();
	p_session->Set(accept_sock, ntohl(client_addr.sin_addr.s_addr), ntohs(client_addr.sin_port), id);
	session_map[id] = p_session;

	// Log
	printf("[SOCKET : %d, id : %d] ACCEPT \n", (int)session_map[id]->sock, session_map[id]->id);

	// Send to Accept User
	// ...
}

void Close_Proc(session_id id) {
	emplace_disconnect_queue(id);

	// Delete Packet Broadcast
	// ...
}

void Header_Check() {

}

void Recv_Proc(session_id id) {
	auto iter = session_map.find(id);
	if (iter == session_map.end())
		throw;

	Session* p_session = iter->second;

	char tmp_buf[BUF_SIZE];
	auto recv_size = recv(p_session->sock, tmp_buf, BUF_SIZE, NULL);

	if (recv_size == 0) {
		Close_Proc(id);
	}
	else if (recv_size == SOCKET_ERROR) {
		switch (WSAGetLastError()) {
			case WSAECONNRESET: {
				emplace_disconnect_queue(id);
				break;
			}

			default: {
				CRASH();
				break;
			}
		}
	}
	else {
		auto ret_enqueue = p_session->recv_buf.Enqueue(tmp_buf, recv_size);
		if (ret_enqueue != recv_size) {
			if (p_session->recv_buf.Full()) {
				emplace_disconnect_queue(id);
			}
			else {
				CRASH();
			}
		}
	}

	PACKET_HEADER header;

	//////////////////////////////////
	// ��Ŷ ó�� ���ɿ��� Ȯ��
	//////////////////////////////////

	if (p_session->recv_buf.Get_UseSize() < sizeof(PACKET_HEADER))
		return;

	auto header_size = p_session->recv_buf.Peek((char*)&header, sizeof(header));

	if (header.code != 0x89) {
		emplace_disconnect_queue(id);
		return;
	}
	if (p_session->recv_buf.Get_UseSize() < sizeof(PACKET_HEADER) + header.payloadSize) {
		return;
	}

	//////////////////////////////////
	// PacketProc ȣ��
	//////////////////////////////////

	p_session->recv_buf.Move_Front(header_size);

	ProtocolBuffer& cs_payload_buf = *protocolBuffer_pool.Alloc();
	cs_payload_buf.Clear();

	auto dequeue_size = p_session->recv_buf.Dequeue(cs_payload_buf.write_pos, p_session->recv_buf.Get_UseSize());
	cs_payload_buf.Move_Wp(dequeue_size);

	PacketProc(id, header.msg_type, cs_payload_buf);
	protocolBuffer_pool.Free(&cs_payload_buf);
}

void Send_Proc(session_id id) {
	auto iter = session_map.find(id);
	if (iter == session_map.end())
		throw;

	Session* p_session = iter->second;

	char tmp_buf[BUF_SIZE];
	auto ret_peek = p_session->send_buf.Peek(tmp_buf, BUF_SIZE);
	auto ret_send = send(p_session->sock, tmp_buf, ret_peek, NULL);

	if (ret_send == SOCKET_ERROR) {
		switch (WSAGetLastError()) {
			case WSAECONNRESET: {
				emplace_discon777nect_queue(id);
				break;
			}

			default: {
				CRASH();
				break;
			}
		}
	}
	p_session->send_buf.Move_Front(ret_send);
}

void Update_Logic() {
	if (delta_time < FRAME_TIME)
		return;

	delta_time -= FRAME_TIME;

	// Logic
	// ...
}

int Unicast(session_id id, PACKET_HEADER* header, const char* packet) {
	auto iter = session_map.find(id);
	if (iter == session_map.end())
		throw;

	Session* p_session = iter->second;

	auto ret_enqueue = p_session->send_buf.Enqueue(header, sizeof(PACKET_HEADER));
	if (ret_enqueue != sizeof(PACKET_HEADER)) {
		if (p_session->send_buf.Full()) {
			emplace_disconnect_queue(id);
			return 0;
		}
		else {
			CRASH();
		}
	}

	ret_enqueue = p_session->send_buf.Enqueue(packet, header->payloadSize);
	if (ret_enqueue != header->payloadSize) {
		if (p_session->send_buf.Full()) {
			emplace_disconnect_queue(id);
			return 0;
		}
		else {
			CRASH();
		}
	}

	return sizeof(PACKET_HEADER) + header->payloadSize;
}

// �������� ����?
void Broadcast(PACKET_HEADER* header, const char* packet, list<session_id>& excludes ) {
	for (auto iter = session_map.begin(); iter != session_map.end(); iter++) {

		// ���ܴ�� �˻�
		for (auto ex_iter = excludes.begin(); ex_iter != excludes.end(); ) {
			if (*ex_iter == iter->first) {
				excludes.erase(ex_iter);
				goto next;
			}
			else
				ex_iter++;
		}

		Unicast(iter->second->id, header, packet);
	next:;
	}
}

void Broadcast(PACKET_HEADER* header, const char* packet) {
	for (auto iter = session_map.begin(); iter != session_map.end(); iter++) {
		Unicast(iter->second->id, header, packet);
	}
}

//////////////////////////////////
// ��Ŷó�� �Լ���
//////////////////////////////////

void MakeCheckSum(PACKET_HEADER* header, const char* payload) {
	int checkSum = header->msg_type;

	for (int i = 0; i < header->payloadSize; i++) {
		checkSum += *payload;
		payload++;
	}

	header->checkSum = checkSum % 256;
}

void PacketProc(session_id id, short msg_type, ProtocolBuffer& cs_payload) {
	PACKET_HEADER header;
	ProtocolBuffer& payload = *protocolBuffer_pool.Alloc();
	payload.Clear();

	switch (msg_type) {
		case REQ_LOGIN: {
			MakePacket_RES_LOGIN(&header, payload, cs_payload, id);
			Unicast(id, &header, payload.Get_readPos());
			break;
		}

		case REQ_ROOM_LIST: {
			MakePacket_RES_ROOM_LIST(&header, payload);
			Unicast(id, &header, payload.Get_readPos());
			break;
		}

		case REQ_ROOM_CREATE: {
			bool result = MakePacket_RES_ROOM_CREATE(&header, payload, cs_payload);
			if (result) {
				Broadcast(&header, payload.Get_readPos());
			}
			else {
				Unicast(id, &header, payload.Get_readPos());
			}
			break;
		}

		case REQ_ROOM_ENTER: {
			MakePacket_RES_ROOM_ENTER(&header, payload, cs_payload, id);
			Unicast(id, &header, payload.Get_readPos());

			auto p_session = Find_Session(id);
			auto room_map_iter = chatRoom_map.find(p_session->no);
		
			// ���� �濡 �ִ� �ο��鿡�Ը� Send
			for (auto room_session_iter = room_map_iter->second->session_list.begin(); room_session_iter != room_map_iter->second->session_list.end(); room_session_iter++) {
				if (*room_session_iter == id)
					continue;

				payload.Clear();
				cs_payload.Clear();

				// << ���� ���� �г���
				cs_payload.Put_Data((char*)&p_session->nickName, sizeof(NickName));

				// << ���� ���� id
				cs_payload << id;

				MakePacket_RES_USER_ENTER(&header, payload, cs_payload);
				Unicast(*room_session_iter, &header, payload.Get_readPos());
			}

			break;
		}

		case REQ_CHAT: {
			// ���� �ο����Ը� ������ �����ؾ���
			MakePacket_RES_CHAT(&header, payload, cs_payload, id);

			auto p_session = Find_Session(id);
			auto chatRoom_map_iter = chatRoom_map.find(p_session->no);
			if (chatRoom_map_iter == chatRoom_map.end())
				throw;
			auto p_chatRoom = chatRoom_map_iter->second;

			// ���� ������ �濡 �ִ� ��� �����鿡�� ��Ŷ ����
			for (auto room_sessions_iter = p_chatRoom->session_list.begin(); room_sessions_iter != p_chatRoom->session_list.end(); room_sessions_iter++) {
				if (*room_sessions_iter == id) {
					continue;
				}
				Unicast(*room_sessions_iter, &header, payload.Get_readPos());
			}

			break;
		}

		case REQ_ROOM_LEAVE: {
			MakePacket_RES_ROOM_LEAVE(&header, payload, id);

			auto p_session = Find_Session(id);
			auto exit_room_no = p_session->no;
			auto& room_session_list = *Get_room_session_list(p_session->no);

			// �����Ϸ��� ���� ��� ���ǵ鿡�� Send (�����û ���� ����)
			for (auto iter = room_session_list.begin(); iter != room_session_list.end(); iter++) {
				Unicast(*iter, &header, payload.Get_readPos());
			}

			// ������ ������ ������ ���� ���ٸ�
			if (0 == Exit_room(id)) {
				payload.Clear();

				// �� ����
				auto  chatRoom_map_iter = chatRoom_map.find(exit_room_no);
				if (chatRoom_map_iter != chatRoom_map.end())
					chatRoom_map.erase(exit_room_no);

				// �� ���� Broadcast
				MakePacket_RES_ROOM_DELETE(&header, payload, exit_room_no);
				Broadcast(&header, payload.Get_readPos());
			}

			break;
		}

		default: {
			break;
		}
	}

	protocolBuffer_pool.Free(&payload);
}

//------------------------------------------------------------
// 1 Req �α���
//
//
// WCHAR[15]	: �г��� (�����ڵ�)
//------------------------------------------------------------
//#define REQ_LOGIN	1

// *** �г������� �˻��ؾ��� (���ǿ� �ѹ�)
//------------------------------------------------------------
// 2 Res �α���              
// 
// 1Byte	: ��� (1:OK / 2:�ߺ��г��� / 3:������ʰ� / 4:��Ÿ����)
// 4Byte	: ����� NO
//------------------------------------------------------------
//#define RES_LOGIN	2
bool MakePacket_RES_LOGIN(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload , session_id login_req_id){
	Session* login_req_session = Find_Session(login_req_id);

	//////////////////////////////////
	// cs ��Ŷ ����
	//////////////////////////////////

	// >> �г���
	NickName nickName;
	cs_payload.Get_Data((char*)&nickName, sizeof(nickName));

	//////////////////////////////////
	// ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	bool login_result = false;

	// �ߺ� �г���, �α��� ����
	if (nickName_table.find(&nickName) != nickName_table.end() ) {
		// << �α��� ���
		payload << RESULT_LOGIN_DNICK;
		payload_size += sizeof(RESULT_LOGIN_DNICK);
	}
	else {
		// �ִ� �ο� �ʰ�, �α��� ����
		if (MAX_USER < nickName_table.size() + 1) {
			// << �α��� ���
			payload << RESULT_LOGIN_MAX;
			payload_size += sizeof(RESULT_LOGIN_MAX);
		}
		// �г��� ����, �α��� ����
		else {
			login_result = true;

			NickName& session_nickName = *nickName_pool.Alloc();
			session_nickName = nickName;
			nickName_table.insert(&session_nickName);
			login_req_session->nickName = session_nickName;

			// << �α��� ���
			payload << RESULT_LOGIN_OK;
			payload_size += sizeof(RESULT_LOGIN_OK);
		}
	}

	// ���� id
	payload << (int)login_req_id;
	payload_size += sizeof(int);

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_LOGIN;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());

	return login_result;
}

//------------------------------------------------------------
// 3 Req ��ȭ�� ����Ʈ
//
//	None
//------------------------------------------------------------
//#define REQ_ROOM_LIST 3

//------------------------------------------------------------
// 4 Res ��ȭ�� ����Ʈ
//
//  2Byte	: ����
//  {
//		4Byte : �� No
//		2Byte : �� �̸� size
//		{
//			Size Byte  : �� �̸� (�����ڵ�)
//		}
// 
//		1Byte : �����ο�
//		{
//			WHCAR[15] : �г���
//		}
//	
//	}
//------------------------------------------------------------
//#define RES_ROOM_LIST 4
void MakePacket_RES_ROOM_LIST(PACKET_HEADER* header, ProtocolBuffer& payload) {
	//////////////////////////////////
	// ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	// �� ����
	payload << (short)chatRoom_map.size();
	payload_size += 2;

	for (auto iter = chatRoom_map.begin(); iter != chatRoom_map.end(); iter++) {
		// �� ��ȣ
		payload << (int)iter->second->room_no;
		payload_size += 4;

		// �� �̸� byte
		short room_name_len = wcslen(iter->second->room_name);
		payload << (short)(room_name_len * 2);
		payload_size += 2;

		// �� �̸�
		payload.Put_Data((char*)iter->second->room_name, room_name_len * 2);
		payload_size += room_name_len * 2;

		// ���� �ο�
		auto chat_user_list = iter->second->session_list;
		auto chat_user_num = chat_user_list.size();
		payload << (char)chat_user_num;
		payload_size += 1;

		// ���� �ο� �̸�
		for (auto iter = chat_user_list.begin(); iter != chat_user_list.end(); iter++) {
			payload.Put_Data((char*)(&Find_Session(*iter)->nickName), sizeof(NickName));
			payload_size += sizeof(NickName);
		}
	}

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_LIST;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}

//------------------------------------------------------------
// 5 Req ��ȭ�� ����
//
// 2Byte : �� �̸� size
// {
//		Size  : �� �̸� (�����ڵ�)
// }
//------------------------------------------------------------
//#define REQ_ROOM_CREATE	5
// *** �� �̸����� �˻��ؾ���
//------------------------------------------------------------
// 6 Res ��ȭ�� ���� (���÷�)
//
// 1Byte : ��� (1:OK / 2:���̸� �ߺ� / 3:�����ʰ� / 4:��Ÿ����)
//
// 4Byte : �� No
// 2Byte : �� �̸� byte size
// {
//		Size  : �� �̸� (�����ڵ�)
// }
//------------------------------------------------------------
//#define RES_ROOM_CREATE	6
//
//#define RESULT_ROOM_CREATE_OK			1
//#define RESULT_ROOM_CREATE_DNICK		2
//#define RESULT_ROOM_CREATE_MAX		3
//#define RESULT_ROOM_CREATE_ETC		4
// �� ���� ���� ��ȯ, �� ���� ���� �� ��ε� ĳ��Ʈ. ���� �� ��û�ڿ��� ���� ĳ��Ʈ
bool MakePacket_RES_ROOM_CREATE(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload_buf) {
	//////////////////////////////////
	// cs ��Ŷ ����
	//////////////////////////////////

	// >> �� �̸� size
	short room_name_size;
	cs_payload_buf >> room_name_size;

	// >> �� �̸�
	wchar_t room_name[256];
	wcsncpy_s(room_name, (wchar_t*)cs_payload_buf.Get_readPos(), room_name_size / 2);
	cs_payload_buf.Move_Rp(room_name_size / 2);

	//////////////////////////////////
	// sc ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	// �� �ߺ� �˻�
	bool overlap_check = false;
	for (auto iter = chatRoom_map.begin(); iter != chatRoom_map.end(); iter++) {
		if (0 == wcscmp(iter->second->room_name, room_name)) {
			overlap_check = true;
			break;
		}
	}

	bool room_create_result = false;

	// �ߺ� �� �̸�, �� ���� ����
	if (overlap_check) {
		// << �� ���� ��û ���
		payload << (char)RESULT_ROOM_CREATE_DNICK;
		payload_size += sizeof(char);
	}
	else {
		// �ִ� �� ���� �ʰ�, �� ���� ����
		if (MAX_ROOM_NUM < chatRoom_map.size() + 1) {
			// << �� ���� ��û ���
			payload << (char)RESULT_ROOM_CREATE_MAX;
			payload_size += sizeof(char);
		}
		// �� ���� ����
		else {
			// �� ����
			int room_no = Get_ChatRoom_No();
			auto& room = *chatRoom_pool.Alloc();
			room.Set(room_no, room_name);
			chatRoom_map.insert({ room_no, &room });

			// << �� ���� ��û ���
			payload << (char)RESULT_ROOM_CREATE_OK;
			payload_size += sizeof(char);
			room_create_result = true;

			// << �� NO
			payload << (int)room_no;
			payload_size += sizeof(int);

			// << �� �̸� byte size
			short room_name_size = (short)(wcslen(room.room_name) * 2);
			payload << (short)room_name_size;
			payload_size += sizeof(short);

			// << �� �̸�
			payload.Put_Data((char*)room.room_name, room_name_size);
			payload_size += room_name_size;
		}
	}

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_CREATE;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());

	return room_create_result;
}

//------------------------------------------------------------
// 7 Req ��ȭ�� ����
//
//	4Byte : �� No
//------------------------------------------------------------
//#define REQ_ROOM_ENTER		7
////------------------------------------------------------------
//// 8 Res ��ȭ�� ����
////
//// 1 Byte : ��� (1:OK / 2:��No ���� / 3:�ο��ʰ� / 4:��Ÿ����)
//// 
////	4Byte : �� No
////	2Byte : �� �̸� size
////	{
////		Size  : �� �̸� (�����ڵ�)
////	}
////	
////	1Byte : �����ο�
////	{
////		WHCAR[15] : �г���
//			4byte : ������ id
////	}
////
////------------------------------------------------------------
//#define RES_ROOM_ENTER		8
//
//#define RESULT_ROOM_ENTER_OK		1
//#define RESULT_ROOM_ENTER_NOT		2
//#define RESULT_ROOM_ENTER_MAX		3
//#define RESULT_ROOM_ENTER_ETC		4
bool MakePacket_RES_ROOM_ENTER(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload, session_id id) {
	Session* p_session = Find_Session(id);

	//////////////////////////////////
	// cs ��Ŷ ����
	//////////////////////////////////

	// >> ���� ��û �� ��ȣ
	int room_no;
	cs_payload >> room_no;

	//////////////////////////////////
	// sc ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	bool enter_result = false;

	auto room_iter = chatRoom_map.find(room_no);
	if (room_iter == chatRoom_map.end())
		throw;

	// �� ���� ����, �� ����
	if (room_iter == chatRoom_map.end()) {
		// << �� ���� ���, ���� �� ����
		payload << (char)RESULT_ROOM_ENTER_NOT;
		payload_size += sizeof(char);
	}
	else {
		auto& room = *room_iter->second;

		// ��ȭ�� ���� ����, �ο� �ʰ�
		if (MAX_ROOM_SESSION < room.session_list.size() + 1) {
			// << �� ���� ���
			payload << (char)RESULT_ROOM_ENTER_MAX;
			payload_size += sizeof(char);
		}
		// ��ȭ�� ���� ����, �ٸ��濡 �� �ִ� ���
		else if (p_session->no != 0) {
			// << �� ���� ���
			payload << (char)RESULT_ROOM_ENTER_ETC;
			payload_size += sizeof(char);
		}
		// ��ȭ�� ���� ����
		else {
			enter_result = true;

			// ���� �� ����, ��ȭ�� �ο� �߰�
			p_session->no = room_no;
			room.session_list.push_back(id);

			// << �� ���� ���
			payload << (char)RESULT_ROOM_ENTER_OK;
			payload_size += sizeof(char);

			// << �� ��ȣ
			payload << (int)room.room_no;
			payload_size += sizeof(int);

			// << �� �̸� byte size
			short room_name_len = wcslen(room.room_name);
			payload << (short)(room_name_len * 2);
			payload_size += sizeof(short);

			// << �� �̸�
			payload.Put_Data((char*)room.room_name, room_name_len * 2);
			payload_size += room_name_len * 2;

			// << ���� �ο�
			auto chat_user_list = room.session_list;
			auto chat_user_num = chat_user_list.size();
			payload << (char)chat_user_num;
			payload_size += 1;

			for (auto iter = chat_user_list.begin(); iter != chat_user_list.end(); iter++) {
				auto nickName = Find_Session(*iter)->nickName;

				// << ���� �ο� �̸�
				payload.Put_Data((char*)&nickName, sizeof(NickName));
				payload_size += sizeof(NickName);

				// << ���� �ο� id
				payload << (int)(*iter);
				payload_size += sizeof(int);
			}
		}
	}

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_ENTER;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());

	return enter_result;
}

////------------------------------------------------------------
//// 9 Req ä�ü۽�
////
//// 2Byte : �޽��� Size
//// {
////		Size  : ��ȭ����(�����ڵ�)
//// }
////------------------------------------------------------------
//#define REQ_CHAT				9
//
////------------------------------------------------------------
//// 10 Res ä�ü��� (�ƹ����� �� �� ����)  (������ ���� ����)
////
//// 4Byte : �۽��� No
//// 2Byte : �޽��� Size
//// {
////		Size  : ��ȭ����(�����ڵ�)
//// }
////------------------------------------------------------------
void MakePacket_RES_CHAT(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload, session_id id) {
	Session* p_session = Find_Session(id);

	//////////////////////////////////
	// cs ��Ŷ ����
	//////////////////////////////////

	// ������ �������� �ʴ� ���� ��ȣ�� ���� �ִ� ��� -> �߻��ؼ��� �ȵǴ� ����
	auto iter = chatRoom_map.find(p_session->no);
	ChatRoom& room = *iter->second;

	// >> �޽��� size
	short msg_size;
	cs_payload >> msg_size;

	// >> �޽���
	wchar_t msg[256];
	cs_payload.Get_Data((char*)msg, msg_size);

	//////////////////////////////////
	// sc ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	// << �߽���
	payload << (int)id;
	payload_size += sizeof(int);

	// << �޽��� size
	payload << (short)msg_size;
	payload_size += sizeof(short);

	// << �޽���
	payload.Put_Data((char*)msg, msg_size);
	payload_size += msg_size;

	
	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_CHAT;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}


////------------------------------------------------------------
//// 11 Req ������ 
////
//// None
////------------------------------------------------------------
//#define REQ_ROOM_LEAVE		11
//
////------------------------------------------------------------
//// 12 Res ������ (����)
////
//// 4Byte : ����� No
////------------------------------------------------------------
//#define RES_ROOM_LEAVE		12
void MakePacket_RES_ROOM_LEAVE(PACKET_HEADER* header, ProtocolBuffer& payload, session_id id) {
	Session* p_session = Find_Session(id);

	//////////////////////////////////
	// cs ��Ŷ ����
	//////////////////////////////////

	// �Ұ� ����

	//////////////////////////////////
	// sc ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	// << �� ���� ���� ���̵�
	payload << (int)id;
	payload_size += sizeof(int);

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_LEAVE;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}

//------------------------------------------------------------
// 13 Res ����� (����)
//
// 4Byte : �� No
//------------------------------------------------------------
void MakePacket_RES_ROOM_DELETE(PACKET_HEADER* header, ProtocolBuffer& payload, room_no no) {
	//////////////////////////////////
	// sc ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	// << �� ���� ���� ���̵�
	payload << (int)no;
	payload_size += sizeof(int);

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_DELETE;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}

//------------------------------------------------------------
// 14 Res Ÿ ����� ���� (����)
//
// WCHAR[15] : �г���(�����ڵ�)
// 4Byte : ����� No
//------------------------------------------------------------
void MakePacket_RES_USER_ENTER(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& enter_session_data) {
	//////////////////////////////////
	// enter_session_data ����
	//////////////////////////////////

	// >> ���� ���� �г���
	NickName enter_nickName;
	enter_session_data.Get_Data((char*)&enter_nickName, sizeof(enter_nickName));

	// >> ���� ���� ���� id
	session_id id;
	enter_session_data >> id;

	//////////////////////////////////
	// sc ��Ŷ ����
	//////////////////////////////////

	int payload_size = 0;

	// << ���� ���� �г���
	payload.Put_Data((char*)&enter_nickName, sizeof(enter_nickName));
	payload_size += sizeof(enter_nickName);

	// << ���� ���� ���� id
	payload << id;
	payload_size += sizeof(int);

	//////////////////////////////////
	// ��� ����
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_USER_ENTER;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}
