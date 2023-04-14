#include "MonitoringLanServer.h"
#include "MonitoringNetServer.h"
#include "MonitorProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include <iostream>

MonitoringLanServer::MonitoringLanServer(const char* systemFile, const char* server, NetServer* localServer) : NetServer(systemFile, server), monitoringNetServer(localServer){
	// Set DB
	char dbAddr[50];
	int port;
	char loginID[50];
	char password[50];
	char schema[50];
	int loggingTime;
	parser.GetValue(server, "DB_IP", dbAddr);
	parser.GetValue(server, "DB_PORT", &port);
	parser.GetValue(server, "DB_ID", loginID);
	parser.GetValue(server, "DB_PASSWORD", password);
	parser.GetValue(server, "DB_SCHEMA", schema);
	parser.GetValue(server, "DB_LOGTIME", &loggingTime);
	parser.GetValue(server, "MONITOR_LOG_TIME", &monitorLogTime);
	try {
		p_dbConnector = new DBConnector(dbAddr, port, loginID, password, schema, loggingTime);
	}
	catch (DBException& e) {
		CRASH();
	}

	dbEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	dbThread = std::thread([this] {DBWriteFunc(); });
}

MonitoringLanServer::~MonitoringLanServer() {
}

void MonitoringLanServer::OnServerStop() {
}

void MonitoringLanServer::OnClientJoin(SessionId sessionId) {
	// User Alloc
	ServerSession* p_user = serverSessionPool.Alloc();
	p_user->Set(sessionId);

	// UserMap Insert
	serverSessionMapLock.Lock();
	serverSessionMap.insert({ sessionId, p_user });
	serverSessionMapLock.Unlock();
}

void MonitoringLanServer::OnClientLeave(SessionId sessionId) {
	// UserMap Erase
	serverSessionMapLock.Lock();
	auto iter = serverSessionMap.find(sessionId);
	ServerSession* p_user = iter->second;
	serverSessionMap.erase(iter);
	serverSessionMapLock.Unlock();

	// User Free
	p_user->Reset();
	serverSessionPool.Free(p_user);
} 

void MonitoringLanServer::OnRecv(SessionId sessionId, PacketBuffer* csContentsPacket){
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
		case en_PACKET_SS_MONITOR_LOGIN: {
			int serverNo;
			try {
				*csContentsPacket >> serverNo;
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(sessionId);
				return;
			}

			// User Find
			serverSessionMapLock.SharedLock();
			auto iter = serverSessionMap.find(sessionId);
			ServerSession* p_user = iter->second;
			serverSessionMapLock.ReleaseSharedLock();

			// 이미 로그인 되있던 유저
			if (p_user->isLogin == true) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // Duplicate LOGIN");
				Disconnect(p_user->sessionID);
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
				*csContentsPacket >> dataType;
				*csContentsPacket >> dataValue;
				*csContentsPacket >> timeStamp;
			}
			catch (const PacketException& e) {
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // impossible : >>");
				Disconnect(sessionId);
				return;
			}

			// User Find
			serverSessionMapLock.SharedLock();
			auto iter = serverSessionMap.find(sessionId);
			ServerSession* p_user = iter->second;
			serverSessionMapLock.ReleaseSharedLock();

			// 세션 로그인 판단 (비정상적 세션 판단)
			if (p_user->isLogin == false) {
				Disconnect(p_user->sessionID);
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // NOT LOGIN");
				return;
			}

			// * MonitoringNetSever로 모니터링 툴에게 모니터링 데이터 송신
			((MonitoringNetServer*)monitoringNetServer)->BroadcastMonitoringData(
				p_user->serverNo, dataType, dataValue, timeStamp);
			SaveMonitoringData(p_user->serverNo, dataType, dataValue, timeStamp);
			return;
		}
		default: {
			LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // INVALID Packet type : %d", type);
			Disconnect(sessionId);
			break;
		}
	}
}

void MonitoringLanServer::DBWriteFunc(){
	for (;;) {
		WaitForSingleObject(dbEvent, INFINITE);
		for (;;) {
			if (dbQ.GetUseCount() < 1) break;
			MonitorData* p_logData;
			dbQ.Dequeue(&p_logData);

			// DB Write
			p_dbConnector->Query(
				"INSERT INTO logdb.monitorlog (serverno, type, avr, min, max) VALUES (%d, %d, %d, %d, %d)",
				p_logData->serverNo, p_logData->dataType, p_logData->sum / p_logData->count, p_logData->min, p_logData->max);
			monitorDataPool.Free(p_logData);
		}
	}
}

void MonitoringLanServer::SaveMonitoringData(int serverNo, int dataType, int data, int timeStamp){
	// Set key
	MonitorKey key;
	key.keyTuple.serverNo = serverNo;
	key.keyTuple.dataType = dataType;

	// update data
	MonitorData* p_data;
	auto iter = monitorDataMap.find(key.keyValue);
	if (iter == monitorDataMap.end()) {
		p_data = monitorDataPool.Alloc();
		p_data->Set(serverNo, dataType, data, timeStamp);
		monitorDataMap.insert({ key.keyValue, p_data });
	}
	else {
		p_data = iter->second;
		p_data->updateData(data);
	}

	// DB Write (10분 주기)
	if (p_data->lastWriteTime + monitorLogTime <= timeStamp) {
		DBJobQueuing(p_data);
		p_data->Init(timeStamp);
	}
}

void MonitoringLanServer::DBJobQueuing(MonitorData* p_data) {
	auto p_logData = monitorDataPool.Alloc();
	*p_logData = *p_data;
	dbQ.Enqueue(p_logData);
	SetEvent(dbEvent);
}

