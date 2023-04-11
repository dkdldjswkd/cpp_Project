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
	// user
	LFObjectPool<MonitorTool> userPool;
	std::unordered_map<DWORD64, MonitorTool*> userMap;
	RecursiveLock userMapLock;

	// protocol
	char loginKey[32];

private:
	// Lib callback (Override)
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }
	void OnClientJoin(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contents_packet);
	void OnClientLeave(SessionId sessionId);
	void OnServerStop();

public:
	void BroadcastMonitoringData(BYTE serverNo, BYTE dataType, int dataValue, int timeStamp);
};