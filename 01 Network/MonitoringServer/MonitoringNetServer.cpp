#include "MonitoringNetServer.h"
#include "MonitorProtocol.h"
#include "../../00 lib_jy/Logger.h"

MonitoringNetServer::MonitoringNetServer(const char* systemFile, const char* server) : NetServer(systemFile, server) {
	parser.GetValue("MonitoringNetServer", "LOGIN_KEY", loginKey);
}

MonitoringNetServer::~MonitoringNetServer() {
}

void MonitoringNetServer::OnClientJoin(SessionId sessionId) {
	// User Alloc
	MonitorTool* p_user = userPool.Alloc();
	p_user->Set(sessionId);

	// UserMap Insert
	userMapLock.Lock();
	userMap.insert({ sessionId, p_user });
	userMapLock.Unlock();
}

void MonitoringNetServer::OnClientLeave(SessionId sessionId) {
	// UserMap Erase
	userMapLock.Lock();
	auto iter = userMap.find(sessionId);
	MonitorTool* p_user = iter->second;
	userMap.erase(iter);
	userMapLock.Unlock();

	// User Free
	p_user->Reset();
	userPool.Free(p_user);
} 

void MonitoringNetServer::OnRecv(SessionId sessionId, PacketBuffer* csContentsPacket){
	WORD type;
	try {
		*csContentsPacket >> type;
	}
	catch (const PacketException& e) {
		LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
		Disconnect(sessionId);
		return;
	}

	switch (type) {
		case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN: {
			char loginSessionKey[32];
			try {
				csContentsPacket->GetData(loginSessionKey, 32);
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(sessionId);
				return;
			}

			// User Find
			userMapLock.SharedLock();
			auto iter = userMap.find(sessionId);
			MonitorTool* p_user = iter->second;
			userMapLock.ReleaseSharedLock();

			// Key 불일치
			if (0 != strncmp(loginKey, loginSessionKey, 32)) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Tool Key Miss (No Disconnect)");

				// Send en_PACKET_CS_MONITOR_TOOL_RES_LOGIN
				PacketBuffer* p_packet = PacketBuffer::Alloc();
				*p_packet << (WORD)en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
				*p_packet << (BYTE)dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY;
				SendPacket(sessionId, p_packet);
				PacketBuffer::Free(p_packet);
				return;
			}

			// 로그인 성공
			p_user->isLogin = true;

			// Send en_PACKET_CS_MONITOR_TOOL_RES_LOGIN
			PacketBuffer* p_packet = PacketBuffer::Alloc();
			*p_packet << (WORD)en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
			*p_packet << (BYTE)dfMONITOR_TOOL_LOGIN_OK;
			SendPacket(sessionId, p_packet);
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
	userMapLock.SharedLock();
	auto userMapEnd = userMap.end();
	for (auto iter = userMap.begin(); iter != userMapEnd; ++iter) {
		auto* p_user = iter->second;
		if (p_user->isLogin) {
			SendPacket(p_user->sessionID, p_packet);
		}
	}
	userMapLock.ReleaseSharedLock();

	PacketBuffer::Free(p_packet);
	return;
}