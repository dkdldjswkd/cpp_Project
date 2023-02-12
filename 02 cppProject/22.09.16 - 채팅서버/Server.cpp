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

// 해당 세션이 속한 방 반환
room_no Find_SessionRoom(session_id id) {
	auto p_session = Find_Session(id);
	if (p_session == nullptr)
		return 0;

	return p_session->no;
}

// 세션 퇴장 후 방에 존재하는 유저 수 반환
int Exit_room(session_id id) {
	auto p_session = Find_Session(id);

	// chatRoom_map.find 가 end면 안됨
	auto p_chatRoom = chatRoom_map.find(p_session->no)->second;

	auto& room_session_list = p_chatRoom->session_list;

	// 방에서 해당 세션 제거
	for (auto iter = room_session_list.begin(); iter != room_session_list.end(); iter++) {
		if (*iter == id) {
			room_session_list.erase(iter);
		}
	}

	// 세션 입장중인 방 초기화
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

	// 리슨소켓 등록
	FD_SET(listen_sock, &read_set);
	sessionTable_id[socket_count] = 0;
	sessoinTable_socket[socket_count] = listen_sock;
	socket_count++;

	// 모든 세션에 대하여 세션 소켓 SET에 등록
	for (auto iter = session_map.begin(); iter != session_map.end(); iter++) {
		Session* p_session = iter->second;

		sessionTable_id[socket_count] = p_session->id;
		sessoinTable_socket[socket_count] = p_session->sock;

		FD_SET(p_session->sock, &read_set);
		if(!p_session->send_buf.Empty())
			FD_SET(p_session->sock, &write_set);

		socket_count++;

		// 64개의 소켓(세션)에 대하여 Select
		if (FD_SETSIZE <= socket_count) {
			Select_Proc(sessionTable_id, sessoinTable_socket, &read_set, &write_set);

			FD_ZERO(&read_set);
			FD_ZERO(&write_set);
			memset(sessionTable_id, -1, sizeof(int) * FD_SETSIZE);
			memset(sessoinTable_socket, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			socket_count = 0;
		}
	}

	// 64개 미만 소켓(세션)에 대하여 Select
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
	// 패킷 처리 가능여부 확인
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
	// PacketProc 호출
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

// 가변인자 변경?
void Broadcast(PACKET_HEADER* header, const char* packet, list<session_id>& excludes ) {
	for (auto iter = session_map.begin(); iter != session_map.end(); iter++) {

		// 제외대상 검사
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
// 패킷처리 함수들
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
		
			// 기존 방에 있던 인원들에게만 Send
			for (auto room_session_iter = room_map_iter->second->session_list.begin(); room_session_iter != room_map_iter->second->session_list.end(); room_session_iter++) {
				if (*room_session_iter == id)
					continue;

				payload.Clear();
				cs_payload.Clear();

				// << 난입 세션 닉네임
				cs_payload.Put_Data((char*)&p_session->nickName, sizeof(NickName));

				// << 난입 세션 id
				cs_payload << id;

				MakePacket_RES_USER_ENTER(&header, payload, cs_payload);
				Unicast(*room_session_iter, &header, payload.Get_readPos());
			}

			break;
		}

		case REQ_CHAT: {
			// 방의 인원에게만 보내게 수정해야함
			MakePacket_RES_CHAT(&header, payload, cs_payload, id);

			auto p_session = Find_Session(id);
			auto chatRoom_map_iter = chatRoom_map.find(p_session->no);
			if (chatRoom_map_iter == chatRoom_map.end())
				throw;
			auto p_chatRoom = chatRoom_map_iter->second;

			// 나를 제외한 방에 있는 모든 유저들에게 패킷 전송
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

			// 퇴장하려는 방의 모든 세션들에게 Send (퇴장요청 세션 포함)
			for (auto iter = room_session_list.begin(); iter != room_session_list.end(); iter++) {
				Unicast(*iter, &header, payload.Get_readPos());
			}

			// 퇴장한 세션이 마지막 유저 였다면
			if (0 == Exit_room(id)) {
				payload.Clear();

				// 방 삭제
				auto  chatRoom_map_iter = chatRoom_map.find(exit_room_no);
				if (chatRoom_map_iter != chatRoom_map.end())
					chatRoom_map.erase(exit_room_no);

				// 방 삭제 Broadcast
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
// 1 Req 로그인
//
//
// WCHAR[15]	: 닉네임 (유니코드)
//------------------------------------------------------------
//#define REQ_LOGIN	1

// *** 닉네임으로 검색해야함 (세션에 한번)
//------------------------------------------------------------
// 2 Res 로그인              
// 
// 1Byte	: 결과 (1:OK / 2:중복닉네임 / 3:사용자초과 / 4:기타오류)
// 4Byte	: 사용자 NO
//------------------------------------------------------------
//#define RES_LOGIN	2
bool MakePacket_RES_LOGIN(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload , session_id login_req_id){
	Session* login_req_session = Find_Session(login_req_id);

	//////////////////////////////////
	// cs 패킷 분해
	//////////////////////////////////

	// >> 닉네임
	NickName nickName;
	cs_payload.Get_Data((char*)&nickName, sizeof(nickName));

	//////////////////////////////////
	// 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	bool login_result = false;

	// 중복 닉네임, 로그인 실패
	if (nickName_table.find(&nickName) != nickName_table.end() ) {
		// << 로그인 결과
		payload << RESULT_LOGIN_DNICK;
		payload_size += sizeof(RESULT_LOGIN_DNICK);
	}
	else {
		// 최대 인원 초과, 로그인 실패
		if (MAX_USER < nickName_table.size() + 1) {
			// << 로그인 결과
			payload << RESULT_LOGIN_MAX;
			payload_size += sizeof(RESULT_LOGIN_MAX);
		}
		// 닉네임 생성, 로그인 성공
		else {
			login_result = true;

			NickName& session_nickName = *nickName_pool.Alloc();
			session_nickName = nickName;
			nickName_table.insert(&session_nickName);
			login_req_session->nickName = session_nickName;

			// << 로그인 결과
			payload << RESULT_LOGIN_OK;
			payload_size += sizeof(RESULT_LOGIN_OK);
		}
	}

	// 본인 id
	payload << (int)login_req_id;
	payload_size += sizeof(int);

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_LOGIN;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());

	return login_result;
}

//------------------------------------------------------------
// 3 Req 대화방 리스트
//
//	None
//------------------------------------------------------------
//#define REQ_ROOM_LIST 3

//------------------------------------------------------------
// 4 Res 대화방 리스트
//
//  2Byte	: 개수
//  {
//		4Byte : 방 No
//		2Byte : 방 이름 size
//		{
//			Size Byte  : 방 이름 (유니코드)
//		}
// 
//		1Byte : 참여인원
//		{
//			WHCAR[15] : 닉네임
//		}
//	
//	}
//------------------------------------------------------------
//#define RES_ROOM_LIST 4
void MakePacket_RES_ROOM_LIST(PACKET_HEADER* header, ProtocolBuffer& payload) {
	//////////////////////////////////
	// 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	// 방 개수
	payload << (short)chatRoom_map.size();
	payload_size += 2;

	for (auto iter = chatRoom_map.begin(); iter != chatRoom_map.end(); iter++) {
		// 방 번호
		payload << (int)iter->second->room_no;
		payload_size += 4;

		// 방 이름 byte
		short room_name_len = wcslen(iter->second->room_name);
		payload << (short)(room_name_len * 2);
		payload_size += 2;

		// 방 이름
		payload.Put_Data((char*)iter->second->room_name, room_name_len * 2);
		payload_size += room_name_len * 2;

		// 참가 인원
		auto chat_user_list = iter->second->session_list;
		auto chat_user_num = chat_user_list.size();
		payload << (char)chat_user_num;
		payload_size += 1;

		// 참가 인원 이름
		for (auto iter = chat_user_list.begin(); iter != chat_user_list.end(); iter++) {
			payload.Put_Data((char*)(&Find_Session(*iter)->nickName), sizeof(NickName));
			payload_size += sizeof(NickName);
		}
	}

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_LIST;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}

//------------------------------------------------------------
// 5 Req 대화방 생성
//
// 2Byte : 방 이름 size
// {
//		Size  : 방 이름 (유니코드)
// }
//------------------------------------------------------------
//#define REQ_ROOM_CREATE	5
// *** 방 이름으로 검색해야함
//------------------------------------------------------------
// 6 Res 대화방 생성 (수시로)
//
// 1Byte : 결과 (1:OK / 2:방이름 중복 / 3:개수초과 / 4:기타오류)
//
// 4Byte : 방 No
// 2Byte : 방 이름 byte size
// {
//		Size  : 방 이름 (유니코드)
// }
//------------------------------------------------------------
//#define RES_ROOM_CREATE	6
//
//#define RESULT_ROOM_CREATE_OK			1
//#define RESULT_ROOM_CREATE_DNICK		2
//#define RESULT_ROOM_CREATE_MAX		3
//#define RESULT_ROOM_CREATE_ETC		4
// 방 생성 여부 반환, 방 생성 성공 시 브로드 캐스트. 실패 시 요청자에게 유니 캐스트
bool MakePacket_RES_ROOM_CREATE(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload_buf) {
	//////////////////////////////////
	// cs 패킷 분해
	//////////////////////////////////

	// >> 방 이름 size
	short room_name_size;
	cs_payload_buf >> room_name_size;

	// >> 방 이름
	wchar_t room_name[256];
	wcsncpy_s(room_name, (wchar_t*)cs_payload_buf.Get_readPos(), room_name_size / 2);
	cs_payload_buf.Move_Rp(room_name_size / 2);

	//////////////////////////////////
	// sc 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	// 방 중복 검사
	bool overlap_check = false;
	for (auto iter = chatRoom_map.begin(); iter != chatRoom_map.end(); iter++) {
		if (0 == wcscmp(iter->second->room_name, room_name)) {
			overlap_check = true;
			break;
		}
	}

	bool room_create_result = false;

	// 중복 방 이름, 방 생성 실패
	if (overlap_check) {
		// << 방 생성 요청 결과
		payload << (char)RESULT_ROOM_CREATE_DNICK;
		payload_size += sizeof(char);
	}
	else {
		// 최대 방 개수 초과, 방 생성 실패
		if (MAX_ROOM_NUM < chatRoom_map.size() + 1) {
			// << 방 생성 요청 결과
			payload << (char)RESULT_ROOM_CREATE_MAX;
			payload_size += sizeof(char);
		}
		// 방 생성 성공
		else {
			// 방 생성
			int room_no = Get_ChatRoom_No();
			auto& room = *chatRoom_pool.Alloc();
			room.Set(room_no, room_name);
			chatRoom_map.insert({ room_no, &room });

			// << 방 생성 요청 결과
			payload << (char)RESULT_ROOM_CREATE_OK;
			payload_size += sizeof(char);
			room_create_result = true;

			// << 방 NO
			payload << (int)room_no;
			payload_size += sizeof(int);

			// << 방 이름 byte size
			short room_name_size = (short)(wcslen(room.room_name) * 2);
			payload << (short)room_name_size;
			payload_size += sizeof(short);

			// << 방 이름
			payload.Put_Data((char*)room.room_name, room_name_size);
			payload_size += room_name_size;
		}
	}

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_CREATE;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());

	return room_create_result;
}

//------------------------------------------------------------
// 7 Req 대화방 입장
//
//	4Byte : 방 No
//------------------------------------------------------------
//#define REQ_ROOM_ENTER		7
////------------------------------------------------------------
//// 8 Res 대화방 입장
////
//// 1 Byte : 결과 (1:OK / 2:방No 오류 / 3:인원초과 / 4:기타오류)
//// 
////	4Byte : 방 No
////	2Byte : 방 이름 size
////	{
////		Size  : 방 이름 (유니코드)
////	}
////	
////	1Byte : 참여인원
////	{
////		WHCAR[15] : 닉네임
//			4byte : 참여자 id
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
	// cs 패킷 분해
	//////////////////////////////////

	// >> 입장 요청 방 번호
	int room_no;
	cs_payload >> room_no;

	//////////////////////////////////
	// sc 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	bool enter_result = false;

	auto room_iter = chatRoom_map.find(room_no);
	if (room_iter == chatRoom_map.end())
		throw;

	// 방 입장 실패, 방 없음
	if (room_iter == chatRoom_map.end()) {
		// << 방 입장 결과, 실패 방 없음
		payload << (char)RESULT_ROOM_ENTER_NOT;
		payload_size += sizeof(char);
	}
	else {
		auto& room = *room_iter->second;

		// 대화방 입장 실패, 인원 초과
		if (MAX_ROOM_SESSION < room.session_list.size() + 1) {
			// << 방 입장 결과
			payload << (char)RESULT_ROOM_ENTER_MAX;
			payload_size += sizeof(char);
		}
		// 대화방 입장 실패, 다른방에 들어가 있는 경우
		else if (p_session->no != 0) {
			// << 방 입장 결과
			payload << (char)RESULT_ROOM_ENTER_ETC;
			payload_size += sizeof(char);
		}
		// 대화방 입장 성공
		else {
			enter_result = true;

			// 세션 방 배정, 대화방 인원 추가
			p_session->no = room_no;
			room.session_list.push_back(id);

			// << 방 입장 결과
			payload << (char)RESULT_ROOM_ENTER_OK;
			payload_size += sizeof(char);

			// << 방 번호
			payload << (int)room.room_no;
			payload_size += sizeof(int);

			// << 방 이름 byte size
			short room_name_len = wcslen(room.room_name);
			payload << (short)(room_name_len * 2);
			payload_size += sizeof(short);

			// << 방 이름
			payload.Put_Data((char*)room.room_name, room_name_len * 2);
			payload_size += room_name_len * 2;

			// << 참여 인원
			auto chat_user_list = room.session_list;
			auto chat_user_num = chat_user_list.size();
			payload << (char)chat_user_num;
			payload_size += 1;

			for (auto iter = chat_user_list.begin(); iter != chat_user_list.end(); iter++) {
				auto nickName = Find_Session(*iter)->nickName;

				// << 참가 인원 이름
				payload.Put_Data((char*)&nickName, sizeof(NickName));
				payload_size += sizeof(NickName);

				// << 참가 인원 id
				payload << (int)(*iter);
				payload_size += sizeof(int);
			}
		}
	}

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_ENTER;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());

	return enter_result;
}

////------------------------------------------------------------
//// 9 Req 채팅송신
////
//// 2Byte : 메시지 Size
//// {
////		Size  : 대화내용(유니코드)
//// }
////------------------------------------------------------------
//#define REQ_CHAT				9
//
////------------------------------------------------------------
//// 10 Res 채팅수신 (아무때나 올 수 있음)  (나에겐 오지 않음)
////
//// 4Byte : 송신자 No
//// 2Byte : 메시지 Size
//// {
////		Size  : 대화내용(유니코드)
//// }
////------------------------------------------------------------
void MakePacket_RES_CHAT(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& cs_payload, session_id id) {
	Session* p_session = Find_Session(id);

	//////////////////////////////////
	// cs 패킷 분해
	//////////////////////////////////

	// 유저가 존재하지 않는 방의 번호를 갖고 있는 경우 -> 발생해서는 안되는 문제
	auto iter = chatRoom_map.find(p_session->no);
	ChatRoom& room = *iter->second;

	// >> 메시지 size
	short msg_size;
	cs_payload >> msg_size;

	// >> 메시지
	wchar_t msg[256];
	cs_payload.Get_Data((char*)msg, msg_size);

	//////////////////////////////////
	// sc 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	// << 발신자
	payload << (int)id;
	payload_size += sizeof(int);

	// << 메시지 size
	payload << (short)msg_size;
	payload_size += sizeof(short);

	// << 메시지
	payload.Put_Data((char*)msg, msg_size);
	payload_size += msg_size;

	
	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_CHAT;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}


////------------------------------------------------------------
//// 11 Req 방퇴장 
////
//// None
////------------------------------------------------------------
//#define REQ_ROOM_LEAVE		11
//
////------------------------------------------------------------
//// 12 Res 방퇴장 (수시)
////
//// 4Byte : 사용자 No
////------------------------------------------------------------
//#define RES_ROOM_LEAVE		12
void MakePacket_RES_ROOM_LEAVE(PACKET_HEADER* header, ProtocolBuffer& payload, session_id id) {
	Session* p_session = Find_Session(id);

	//////////////////////////////////
	// cs 패킷 분해
	//////////////////////////////////

	// 할거 없음

	//////////////////////////////////
	// sc 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	// << 방 퇴장 세션 아이디
	payload << (int)id;
	payload_size += sizeof(int);

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_LEAVE;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}

//------------------------------------------------------------
// 13 Res 방삭제 (수시)
//
// 4Byte : 방 No
//------------------------------------------------------------
void MakePacket_RES_ROOM_DELETE(PACKET_HEADER* header, ProtocolBuffer& payload, room_no no) {
	//////////////////////////////////
	// sc 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	// << 방 퇴장 세션 아이디
	payload << (int)no;
	payload_size += sizeof(int);

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_ROOM_DELETE;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}

//------------------------------------------------------------
// 14 Res 타 사용자 입장 (수시)
//
// WCHAR[15] : 닉네임(유니코드)
// 4Byte : 사용자 No
//------------------------------------------------------------
void MakePacket_RES_USER_ENTER(PACKET_HEADER* header, ProtocolBuffer& payload, ProtocolBuffer& enter_session_data) {
	//////////////////////////////////
	// enter_session_data 분해
	//////////////////////////////////

	// >> 난입 유저 닉네임
	NickName enter_nickName;
	enter_session_data.Get_Data((char*)&enter_nickName, sizeof(enter_nickName));

	// >> 난입 유저 세션 id
	session_id id;
	enter_session_data >> id;

	//////////////////////////////////
	// sc 패킷 생성
	//////////////////////////////////

	int payload_size = 0;

	// << 난입 유저 닉네임
	payload.Put_Data((char*)&enter_nickName, sizeof(enter_nickName));
	payload_size += sizeof(enter_nickName);

	// << 난입 유저 세션 id
	payload << id;
	payload_size += sizeof(int);

	//////////////////////////////////
	// 헤더 생성
	//////////////////////////////////
	header->code = PACKET_CODE;
	header->msg_type = RES_USER_ENTER;
	header->payloadSize = payload_size;
	MakeCheckSum(header, payload.Get_readPos());
}
