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
	static union MonitorKey {
		struct {
			WORD serverNo;
			WORD dataType;
		} keyTuple;
		DWORD keyValue;
	};
	static struct MonitorData {
		WORD serverNo;
		WORD dataType;
		DWORD count = 0;
		DWORD64 sum = 0;
		DWORD min = MAXDWORD;
		DWORD max = 0;
		DWORD lastWriteTime = 0;

	public:
		void updateData(DWORD64 data) {
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
		void Set(WORD serverNo, WORD dataType, DWORD64 data, DWORD lastWriteTime) {
			this->serverNo = serverNo;
			this->dataType = dataType;
			Init(lastWriteTime);
			updateData(data);
		}
		MonitorData& operator=(const MonitorData& other) {
			this->serverNo = other.serverNo;
			this->dataType = other.dataType;
			this->count = other.count;
			this->sum = other.sum;
			this->min = other.min;
			this->max = other.max;
			this->lastWriteTime = other.lastWriteTime;
			return *this;
		}
	};

private:
	// User
	LFObjectPool<ServerSession> serverSessionPool;
	std::unordered_map<DWORD64, ServerSession*> serverSessionMap;
	RecursiveLock serverSessionMapLock;

	// DB
	DBConnector* p_dbConnector;
	std::thread dbThread;
	bool dbStop = false;
	HANDLE dbEvent;
	LFQueue<MonitorData*> dbQ;

	// Monitor Data
	LFObjectPool<MonitorData> monitorDataPool;
	std::unordered_map<DWORD, MonitorData*> monitorDataMap;

	// Control Server (this´Â Contol ServerÀÇ agent)
	NetServer* monitoringNetServer;

	// opt
	int monitorLogTime;

private:
	// Lib callback (NetServer Override)
	bool OnConnectionRequest(in_addr IP, WORD Port) { return true; }
	void OnClientJoin(SessionId sessionId);
	void OnClientLeave(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contents_packet);
	void OnServerStop();

private:
	// DB
	void SaveMonitoringData(int serverNo, int dataTyp, int data, int timeStamp);
	void DBWriteFunc();
	void DBJobQueuing(MonitorData* p_data);
};