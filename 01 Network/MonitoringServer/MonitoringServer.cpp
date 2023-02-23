#include "MonitoringServer.h"
#include "MonitorProtocol.h"
#include "../NetworkLib/Logger.h"

MonitoringServer::MonitoringServer(const char* systemFile, const char* server) : NetworkLib(systemFile, server) {
	parser.GetValue("MonitoringServer", "LOGIN_KEY", loginKey);
}

MonitoringServer::~MonitoringServer() {
}

bool MonitoringServer::OnConnectionRequest(in_addr IP, WORD Port){
	return false;
}

void MonitoringServer::OnClientJoin(SESSION_ID session_id){
	// User Alloc
	 User* p_user = userPool.Alloc();
	 p_user->Set(session_id);

	// UserMap Insert
	 userMapLock.Lock_Exclusive();
	 userMap.insert({ session_id, p_user });
	 userMapLock.Unlock_Exclusive();
}

void MonitoringServer::OnClientLeave(SESSION_ID session_id) {
	// UserMap Erase
	userMapLock.Lock_Exclusive();
	auto iter = userMap.find(session_id);
	User* p_user = iter->second;
	userMap.erase(iter);
	userMapLock.Unlock_Exclusive();

	// User Free
	p_user->Reset();
	userPool.Free(p_user);
} 

void MonitoringServer::OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet){
	WORD type;
	try {
		*contents_packet >> type;
	}
	catch (const PacketException& e) {
		LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(session_id);
		return;
	}
	ProcPacket(session_id, type, contents_packet);
}

void MonitoringServer::ProcPacket(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket) {
	switch (type) {
		case en_PACKET_SS_MONITOR_LOGIN: {
			int serverNo;
			try {
				*cs_contentsPacket >> serverNo;
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(session_id);
				return;
			}

			// User Find
			userMapLock.Lock_Shared();
			auto iter = userMap.find(session_id);
			User* p_user = iter->second;
			userMapLock.Unlock_Shared();

			// 이미 로그인 되있던 유저
			if (p_user->is_login == true) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // Duplicate LOGIN");
				Disconnect(p_user->session_id);
				return;
			}

			// 로그인 성공
			p_user->LoginFromServer(serverNo);
			return;
		}

		case en_PACKET_SS_MONITOR_DATA_UPDATE: {
			BYTE dataType; // 모니터링 데이터 타입
			int	dataValue;
			int	timeStamp;
			try {
				*cs_contentsPacket >> dataType;
				*cs_contentsPacket >> dataValue;
				*cs_contentsPacket >> timeStamp;
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(session_id);
				return;
			}

			// User Find
			userMapLock.Lock_Shared();
			auto iter = userMap.find(session_id);
			User* p_user = iter->second;
			userMapLock.Unlock_Shared();

			// 로그인 안된 유저 Disconnect
			if (p_user->is_login == false) {
				Disconnect(p_user->session_id);
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // NOT LOGIN");
				return;
			}

			// Send en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE
			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << (WORD)en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;
			*p_packet << (BYTE)p_user->serverNo;
			*p_packet << (BYTE)dataValue;
			*p_packet << (INT64)timeStamp;
			SendPacket(session_id, p_packet);
			PacketBuffer::Free(p_packet);
			return;
		}

		case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN: {
			char loginSessionKey[32];
			try {
				cs_contentsPacket->Get_Data(loginSessionKey, 32);
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(session_id);
				return;
			}

			// User Find
			userMapLock.Lock_Shared();
			auto iter = userMap.find(session_id);
			User* p_user = iter->second;
			userMapLock.Unlock_Shared();

			// Key 불일치
			if (0 != strncmp(loginKey, loginSessionKey, 32) ){
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Tool Key Miss (No Disconnect)");

				// Send en_PACKET_CS_MONITOR_TOOL_RES_LOGIN
				PacketBuffer* p_packet = PacketBuffer::Alloc();
				*p_packet << (WORD)en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
				*p_packet << (BYTE)dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY;
				SendPacket(session_id, p_packet);
				PacketBuffer::Free(p_packet);
				return;
			}

			// 로그인 성공
			p_user->LoginFromTool();

			// Send en_PACKET_CS_MONITOR_TOOL_RES_LOGIN
			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << (WORD)en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
			*p_packet << (BYTE)dfMONITOR_TOOL_LOGIN_OK;
			SendPacket(session_id, p_packet);
			PacketBuffer::Free(p_packet);
			return;
		}
	}
}