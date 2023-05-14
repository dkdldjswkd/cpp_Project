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
	// Set 로그인 성공 시 회신해줄 연결서버 IP, Port
	parser.GetValue(server, "CHATTING_SERVER_IP", chatServerIP);
	parser.GetValue(server, "CHATTING_SERVER_PORT", (int*)&ChatServerPort);
	parser.GetValue(server, "GAME_SERVER_IP", gameServerIP);
	parser.GetValue(server, "GAME_SERVER_PORT", (int*)&gameServerPort);
	
	p_connecterTLS = new DBConnectorTLS(dbAddr, port, loginID, password, schema, loggingTime);
	connectorRedis.connect();
}

LoginServer::~LoginServer(){
	delete p_connecterTLS;
}

bool LoginServer::OnConnectionRequest(in_addr IP, WORD Port){
	// ex. printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
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
	// User map 삭제
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

			// DB 조회 (DB의 sessionKey와 패킷의 sessionKey 비교를 위함)
			sqlResult = p_connecterTLS->Query("SELECT sessionkey FROM sessionkey WHERE accountno = %d", accountNo);
			sqlRow = mysql_fetch_row(sqlResult);
			if (NULL == sqlRow) {
				Disconnect(sessionId);
				mysql_free_result(sqlResult);
				return;
			}

			// 인증 판단
			// ...

			mysql_free_result(sqlResult);

			// DB 조회 (userid, usernick 등을 담은 패킷을 회신하기 위함)
			sqlResult = p_connecterTLS->Query("SELECT userid, usernick FROM account WHERE accountno = %d", accountNo);
			sqlRow = mysql_fetch_row(sqlResult);
			WCHAR id[20];
			UTF8ToUTF16(sqlRow[0], id);
			WCHAR nickname[20];
			UTF8ToUTF16(sqlRow[1], nickname);
			mysql_free_result(sqlResult);

			// Set IP, PORT
			WCHAR	GameServerIP[16];
			WCHAR	ChatServerIP[16];
			UTF8ToUTF16(this->gameServerIP, GameServerIP);
			UTF8ToUTF16(this->chatServerIP, ChatServerIP);
			USHORT	GameServerPort = this->gameServerPort;
			USHORT	ChatServerPort = this->ChatServerPort;

			// 로그인 응답 패킷 생성
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

			// Redis에 인증토큰 삽입
			char GameKey[100];		// 게임서버에서 조회할 key
			char chattingKey[100];	// 채팅서버에서 조회할 key
			snprintf(chattingKey, 100, "%lld.chatting", accountNo);
			snprintf(GameKey, 100, "%lld.game", accountNo);
			connectorRedis.setex(chattingKey, 10, (char*)&sessoinKey);
			connectorRedis.setex(GameKey, 10, (char*)&sessoinKey);
			connectorRedis.sync_commit();

			// 로그인 응답 패킷 송신
			SendPacket(sessionId, p_packet);
			PacketBuffer::Free(p_packet);
			break;
		}
		default: {
			LOG("LoginServer", LOG_LEVEL_WARN, "Disconnect // OnRecv() : INVALID Packet type (%d)", type);
			Disconnect(sessionId);
			break;
		}
	}
}