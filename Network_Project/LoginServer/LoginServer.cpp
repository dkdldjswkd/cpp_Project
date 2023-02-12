#include "LoginServer.h"
#include "CommonProtocol.h"
#include "NetworkLib\Logger.h"
using namespace J_LIB;
using namespace std;

LoginServer::LoginServer(){
}

LoginServer::~LoginServer(){
}

bool LoginServer::OnConnectionRequest(in_addr IP, WORD Port){
	// printf("[Accept] IP(%s), PORT(%u) \n", inet_ntoa(ip), port);
	return true;
}

void LoginServer::OnClientJoin(SESSION_ID session_id) {
	Player* p_player = playerPool.Alloc();
	playerMap.insert({ session_id, p_player });
}

void LoginServer::OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet){
	WORD type;
	try {
		*contents_packet >> type;
	}
	catch (const PacketException& e) {
		LOG("LoginServer", LOG_LEVEL_WARN, "impossible : >> type");
		Disconnect(session_id);
		return;
	}

	// INVALID Packet type
	if (type == en_PACKET_CS_LOGIN_REQ_LOGIN) { // 메시지 타입은 하나만 존재
		LOG("LoginServer", LOG_LEVEL_WARN, "OnRecv() : INVALID Packet type (%d)", type);
		Disconnect(session_id);
		return;
	}

	// DB 조회

}

void LoginServer::OnClientLeave(SESSION_ID session_id){
}