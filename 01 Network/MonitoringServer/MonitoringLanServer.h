#pragma once
#include <unordered_map>
#include <thread>
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/RecursiveLock.h"
#include "../../00 lib_jy/LFObjectPool.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../DBConnector/DBConnector.h"
#include "ServerSession.h"

class MonitoringLanServer : public NetServer {
public:
	MonitoringLanServer(const char* systemFile, const char* server, NetServer* localServer);
	~MonitoringLanServer();

private:
	static struct MonitoredData {
		DWORD count = 0;
		DWORD64 sum = 0;
		DWORD min = MAXDWORD;
		DWORD max = 0;
		DWORD lastWriteTime = 0;

	public:
		MonitoredData() {}
		MonitoredData(DWORD64 data, DWORD lastWriteTime) { AddData(data); this->lastWriteTime = lastWriteTime; }
	public:
		void AddData(DWORD64 data) {
			++count;
			sum += data;
			if (data < min) min = data;
			else if (max < data) max = data;
		}
		void Init(DWORD lastWriteTime) {
			count = 0;
			sum = 0;
			min = MAXDWORD;
			max = 0;
			this->lastWriteTime = lastWriteTime;
		}
	};
	// <dataType, MonitoredData>
	typedef std::unordered_map<int, MonitoredData*> MonitoredDataMap;
	// <serverNo, MonitoredDataMap>
	typedef std::unordered_map <int, MonitoredDataMap*> ServerMonitoredDataMap;

	static struct DBJob {
		int serverNo;
		int dataType;
		DWORD avr;
		DWORD min;
		DWORD max;
	};

private:
	// User
	LFObjectPool<ServerSession> serverSessionPool;
	std::unordered_map<DWORD64, ServerSession*> serverSessionMap;
	RecursiveLock serverSessionMapLock;

	// Monitoring
	ServerMonitoredDataMap serverMonitoredDataMap;

	// DB
	DBConnector* p_dbConnector;
	std::thread dbThread;
	HANDLE dbEvent;
	LFQueue<DBJob*> dbJobQ;
	LFObjectPool<DBJob> dbJobPool;

	// Control Server (this´Â Contol ServerÀÇ agent)
	NetServer* localServer;

private:
	// Lib callback (NetServer Override)
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }
	void OnClientJoin(SessionId sessionId);
	void OnClientLeave(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contents_packet);

private:
	// DB
	void SaveMonitoringData(int serverNo, int dataTyp, int data, int timeStamp);
	void DBWriteFunc();
	void DBJobQueuing(int serverNo, int dataType, MonitoredData* p_data);
};