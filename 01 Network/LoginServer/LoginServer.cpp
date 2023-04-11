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
	delete p_connecterTLS;
}

void LoginServer::OnServerStop() {
}

bool LoginServer::OnConnectionRequest(in_addr IP, WORD Port){
	// printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void LoginServer::OnClientJoin(SessionId sessionId) {
	User* p_player = userPool.Alloc();
	p_player->Set(sessionId);

	userMapLock.Lock();
	userMap.insert({ sessionId, p_player });
	userMapLock.Unlock();
}

void LoginServer::OnClientLeave(SessionId sessionId) {
	// User map ����
	userMapLock.Lock();
	auto iter = userMap.find(sessionId);
	User* p_player = iter->second;
	userMap.erase(iter);
	userMapLock.Unlock();

	userPool.Free(p_player);
}

void LoginServer::OnRecv(SessionId sessionId, PacketBuffer* csContentsPacket) {
	WORD type;
	try {
		*csContentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("LoginServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(sessionId);
		return;
	}	
	switch (type) {
		case en_PACKET_CS_LOGIN_REQ_LOGIN: {
			INT64 accountNo;
			SessionKey sessoinKey;
			try {
				*csContentsPacket >> accountNo;
				csContentsPacket->GetData((char*)&sessoinKey, sizeof(SessionKey));
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(sessionId);
				return;
			}

			MYSQL_RES* sqlResult;
			MYSQL_ROW sqlRow;

			// DB ��ȸ
			sqlResult = p_connecterTLS->Query("SELECT sessionkey FROM sessionkey WHERE accountno = %d", accountNo);
			sqlRow = mysql_fetch_row(sqlResult);
			if (NULL == sqlRow) {
				Disconnect(sessionId);
				mysql_free_result(sqlResult);
				return;
			}

			// ���� �Ǵ� (���� �ڵ忡���� ��Ŷ session key, DB session key ���ٰ� �����ϰ� �ڵ�����)
			mysql_free_result(sqlResult);

			// DB ��ȸ (Get userid, usernick)
			sqlResult = p_connecterTLS->Query("SELECT userid, usernick FROM account WHERE accountno = %d", accountNo);
			sqlRow = mysql_fetch_row(sqlResult);
			WCHAR id[20];
			UTF8ToUTF16(sqlRow[0], id);
			WCHAR nickname[20];
			UTF8ToUTF16(sqlRow[1], nickname);
			mysql_free_result(sqlResult);

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

			// Redis�� ������ū ����
			char chattingKey[100]; // ä�ü������� ��ȸ�� key
			char GameKey[100]; // ���Ӽ������� ��ȸ�� key
			snprintf(chattingKey, 100, "%d.chatting", accountNo);
			snprintf(GameKey, 100, "%d.game", accountNo);
			connectorRedis.setex(chattingKey, 10, (char*)&sessoinKey);
			connectorRedis.setex(GameKey, 10, (char*)&sessoinKey);
			connectorRedis.sync_commit();

			// �α��� ���� ��Ŷ �۽�
			SendPacket(sessionId, p_packet);
			PacketBuffer::Free(p_packet);
		}
		default: {
			LOG("LoginServer", LOG_LEVEL_WARN, "Disconnect // OnRecv() : INVALID Packet type (%d)", type);
			Disconnect(sessionId);
			break;
		}
	}
}