#include "LoginServer.h"
#include "CommonProtocol.h"
#include "NetworkLib/Logger.h"
using namespace J_LIB;
using namespace std;

LoginServer::LoginServer(const char* dbAddr, const char* loginID, const char* password, const char* schema, const int port, unsigned short loggingTime)
	: connecterTLS(dbAddr, loginID, password, schema, port, loggingTime) {
}

LoginServer::~LoginServer(){
}

bool LoginServer::OnConnectionRequest(in_addr IP, WORD Port){
	// printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void LoginServer::OnClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc();
	p_player->Set(session_id);
	playerMap.insert({ session_id, p_player });
}

void LoginServer::OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet){
	WORD type;
	INT64 accountNo;
	char sessionKey[64];

	try {
		*contents_packet >> type;
		// INVALID Packet type
		if (type == en_PACKET_CS_LOGIN_REQ_LOGIN) { // �޽��� Ÿ���� �ϳ��� ����
			LOG("LoginServer", LOG_LEVEL_WARN, "OnRecv() : INVALID Packet type (%d)", type);
			Disconnect(session_id);
			return;
		}
		*contents_packet >> accountNo;
		contents_packet->Get_Data(sessionKey, 64);
	}
	catch (const PacketException& e) {
		LOG("LoginServer", LOG_LEVEL_WARN, "impossible : >> type");
		Disconnect(session_id);
		return;
	}

	// DB ��ȸ

}

void LoginServer::OnClientLeave(SESSION_ID session_id){
}