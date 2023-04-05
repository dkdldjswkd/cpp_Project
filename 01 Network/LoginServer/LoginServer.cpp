#include "LoginServer.h"
#include <string>
#include "CommonProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include "../../00 lib_jy/StringUtils.h"
using namespace std;

//------------------------------
// LoginServer
//------------------------------
LoginServer::LoginServer(const char* systemFile, const char* server) : NetServer(systemFile, server){
	char dbAddr[50];
	int port;
	char loginID[50];
	char password[50];
	char schema[50];
	int loggingTime;

	// Set DB
	parser.GetValue(server, "DB_IP", dbAddr);
	parser.GetValue(server, "DB_PORT", &port);
	parser.GetValue(server, "DB_ID", loginID);
	parser.GetValue(server, "DB_PASSWORD", password);
	parser.GetValue(server, "DB_SCHEMA", schema);
	parser.GetValue(server, "DB_LOGTIME", &loggingTime);

	// Set Other Server IP, Port (�ӽù���, ���߿� �ý��� ���Ͽ��� ���� �ܾ�ð�)
	strncpy_s(chatServerIP, 20, "127.0.0.1", strlen("127.0.0.1"));
	strncpy_s(gameServerIP, 20, "127.0.0.1", strlen("127.0.0.1"));
	parser.GetValue("ChattingServer_Single", "PORT", (int*)&ChatServerPort);
	gameServerPort = 2000;

	p_connecterTLS = new DBConnectorTLS(dbAddr, port, loginID, password, schema, loggingTime);
	connectorRedis.connect();
}

LoginServer::~LoginServer(){
	//delete(p_connecterTLS);
}

bool LoginServer::OnConnectionRequest(in_addr IP, WORD Port){
	// printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void LoginServer::OnClientJoin(SESSION_ID session_id) {
	User* p_player = UserPool.Alloc();
	p_player->Set(session_id);

	UserMapLock.Lock_Exclusive();
	UserMap.insert({ session_id, p_player });
	UserMapLock.Unlock_Exclusive();
}

void LoginServer::OnClientLeave(SESSION_ID session_id) {
	// User map ����
	UserMapLock.Lock_Exclusive();
	auto iter = UserMap.find(session_id);
	User* p_player = iter->second;
	UserMap.erase(iter);
	UserMapLock.Unlock_Exclusive();

	UserPool.Free(p_player);
}

void LoginServer::OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet){
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;

	WORD type;
	INT64 accountNo;
	Token token;
	try {
		*contents_packet >> type;
		// INVALID Packet type
		if (type != en_PACKET_CS_LOGIN_REQ_LOGIN) { // �޽��� Ÿ���� �ϳ��� ����
			LOG("LoginServer", LOG_LEVEL_WARN, "Disconnect // OnRecv() : INVALID Packet type (%d)", type);
			Disconnect(session_id);
			return;
		}
		*contents_packet >> accountNo;
		contents_packet->GetData((char*)&token, sizeof(Token));
	}
	catch (const PacketException& e) {
		LOG("LoginServer", LOG_LEVEL_WARN, "Disconnect // impossible : >> type");
		Disconnect(session_id);
		return;
	}

	// DB ��ȸ (������ ������ token�� DB�� token�� ��ġ���� �Ǵ�)
	sql_result = p_connecterTLS->Query("SELECT sessionkey FROM sessionkey WHERE accountno = %d", accountNo);
	sql_row = mysql_fetch_row(sql_result);
	if (NULL == sql_row) return;
	// if strcmp(sessionKey[64], sql_row[0]), DB token�� Packet�� token�� ��ġ�ϴ��� Ȯ�� (������ ��ġ�Ѵٰ� �Ǵ�, sql_row[0] == null)
	mysql_free_result(sql_result);

	// DB ��ȸ (���� ID, Nickname Send �ϱ����� ��ȸ)
	sql_result = p_connecterTLS->Query("SELECT userid, usernick FROM account WHERE accountno=%d", accountNo);
	sql_row = mysql_fetch_row(sql_result);
	WCHAR id[20];
	UTF8ToUTF16(sql_row[0], id);
	WCHAR nickname[20];
	UTF8ToUTF16(sql_row[1], nickname);
	mysql_free_result(sql_result);

	// Set IP, PORT
	WCHAR	GameServerIP[16] = { 0, };
	WCHAR	ChatServerIP[16] = { 0, };
	UTF8ToUTF16(this->gameServerIP, GameServerIP);
	UTF8ToUTF16(this->chatServerIP, ChatServerIP);
	USHORT	GameServerPort = this->gameServerPort;
	USHORT	ChatServerPort = this->ChatServerPort;

	// �α��� ���� ��Ŷ ����
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN;
	*p_packet << (INT64)accountNo;
	*p_packet << (BYTE)dfLOGIN_STATUS_OK;
	p_packet->PutData((char*)id, 40);
	p_packet->PutData((char*)nickname, 40);
	p_packet->PutData((char*)GameServerIP, 32);
	*p_packet << (USHORT)GameServerPort;
	p_packet->PutData((char*)ChatServerIP, 32);
	*p_packet << (USHORT)ChatServerPort;

	// Redis�� ���� ��ū �߰�
	char chattingKey[100]; // ä�ü������� ��ȸ�� key
	char GameKey[100]; // ���Ӽ������� ��ȸ�� key
	snprintf(chattingKey, 100, "%d.chatting", accountNo);
	snprintf(GameKey, 100, "%d.game", accountNo);
	connectorRedis.setex(chattingKey, 10, (char*)&token);
	connectorRedis.setex(GameKey, 10, (char*)&token);
	connectorRedis.sync_commit();
	//LOG("LoginServer", LOG_LEVEL_DEBUG, "Set Token // accountNo(%d) Token(%.*s)", accountNo, sizeof(Token), (char*)&token);

	SendPacket(session_id, p_packet);
	PacketBuffer::Free(p_packet);
}