#include "MonitoringNetServer.h"
#include "MonitorProtocol.h"
#include "../../00 lib_jy/Logger.h"

MonitoringNetServer::MonitoringNetServer(const char* systemFile, const char* server) : NetServer(systemFile, server) {
	parser.GetValue("MonitoringNetServer", "LOGIN_KEY", loginKey);
}

MonitoringNetServer::~MonitoringNetServer() {
}

void MonitoringNetServer::OnClientJoin(SESSION_ID session_id) {
	// User Alloc
	MonitorTool* p_user = userPool.Alloc();
	p_user->Set(session_id);

	// UserMap Insert
	userMapLock.Lock_Exclusive();
	userMap.insert({ session_id, p_user });
	userMapLock.Unlock_Exclusive();
}

void MonitoringNetServer::OnClientLeave(SESSION_ID session_id) {
	// UserMap Erase
	userMapLock.Lock_Exclusive();
	auto iter = userMap.find(session_id);
	MonitorTool* p_user = iter->second;
	userMap.erase(iter);
	userMapLock.Unlock_Exclusive();

	// User Free
	p_user->Reset();
	userPool.Free(p_user);
} 

void MonitoringNetServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
	WORD type;
	try {
		*cs_contentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(session_id);
		return;
	}

	switch (type) {
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
			MonitorTool* p_user = iter->second;
			userMapLock.Unlock_Shared();

			// Key 불일치
			if (0 != strncmp(loginKey, loginSessionKey, 32)) {
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
			p_user->isLogin = true;

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

void MonitoringNetServer::BroadcastMonitoringData(BYTE serverNo, BYTE dataType, int dataValue, int timeStamp){
	// en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE
	PacketBuffer* p_packet = PacketBuffer::Alloc();
	*p_packet << (WORD)en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;
	*p_packet << (BYTE)serverNo;
	*p_packet << (BYTE)dataType;
	*p_packet << (int)dataValue;
	*p_packet << (int)timeStamp;

	// 붙어있는 모니터링 툴에게 브로드 캐스트
	userMapLock.Lock_Shared();
	auto userMapEnd = userMap.end();
	for (auto iter = userMap.begin(); iter != userMapEnd; ++iter) {
		auto* p_user = iter->second;
		if (p_user->isLogin) {
			SendPacket(p_user->sessionID, p_packet);
		}
	}
	userMapLock.Unlock_Shared();

	PacketBuffer::Free(p_packet);
	return;
}