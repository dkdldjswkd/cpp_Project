#include "MonitoringLanServer.h"
#include "MonitoringNetServer.h"
#include "MonitorProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include <iostream>

MonitoringLanServer::MonitoringLanServer(const char* systemFile, const char* server, NetServer* localServer) : NetServer(systemFile, server), localServer(localServer) {
}

MonitoringLanServer::~MonitoringLanServer() {
}

void MonitoringLanServer::OnClientJoin(SESSION_ID session_id) {
	// User Alloc
	ServerSession* p_user = serverSessionPool.Alloc();
	p_user->Set(session_id);

	// UserMap Insert
	serverSessionMapLock.Lock_Exclusive();
	serverSessionMap.insert({ session_id, p_user });
	serverSessionMapLock.Unlock_Exclusive();
}

void MonitoringLanServer::OnClientLeave(SESSION_ID session_id) {
	// UserMap Erase
	serverSessionMapLock.Lock_Exclusive();
	auto iter = serverSessionMap.find(session_id);
	ServerSession* p_user = iter->second;
	serverSessionMap.erase(iter);
	serverSessionMapLock.Unlock_Exclusive();

	// User Free
	p_user->Reset();
	serverSessionPool.Free(p_user);
} 

void MonitoringLanServer::OnRecv(SESSION_ID session_id, PacketBuffer* cs_contentsPacket){
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
			serverSessionMapLock.Lock_Shared();
			auto iter = serverSessionMap.find(session_id);
			ServerSession* p_user = iter->second;
			serverSessionMapLock.Unlock_Shared();

			// 이미 로그인 되있던 유저
			if (p_user->is_login == true) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // Duplicate LOGIN");
				Disconnect(p_user->session_id);
				return;
			}

			// 로그인 성공
			p_user->Login(serverNo);
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
			serverSessionMapLock.Lock_Shared();
			auto iter = serverSessionMap.find(session_id);
			ServerSession* p_user = iter->second;
			serverSessionMapLock.Unlock_Shared();

			// 세션 로그인 판단 (비정상적 세션 판단)
			if (p_user->is_login == false) {
				Disconnect(p_user->session_id);
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // NOT LOGIN");
				return;
			}

			// * MonitoringNetSever로 모니터링 툴에게 모니터링 데이터 송신
			((MonitoringNetServer*)localServer)->BroadcastMonitoringData(p_user->serverNo, dataType, dataValue, timeStamp);
			return;

			// 디버깅
			{
				//printf(
				//	"server no  : %d\n"
				//	"data type  : %d\n"
				//	"data value : %d\n"
				//	"time stamp : %d\n"
				//	,
				//	p_user->serverNo,
				//	dataType,
				//	dataValue,
				//	timeStamp
				//);
			}
		}
	}
}