#pragma once
#include <unordered_map>
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/RecursiveLock.h"
#include "../../00 lib_jy/LFObjectPool.h"
#include "MonitorSession.h"

class MonitoringNetServer : public NetServer {
public:
	MonitoringNetServer(const char* systemFile, const char* server);
	~MonitoringNetServer();

private:
	J_LIB::LFObjectPool<MonitorTool> userPool;
	std::unordered_map<INT64, MonitorTool*> userMap;
	RecursiveLock userMapLock;
	char loginKey[32];

private:
	// Lib callback (Override)
	void OnClientJoin(SESSION_ID session_id);
	void OnClientLeave(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }

public:
	void BroadcastMonitoringData(BYTE serverNo, BYTE dataType, int dataValue, int timeStamp);
};