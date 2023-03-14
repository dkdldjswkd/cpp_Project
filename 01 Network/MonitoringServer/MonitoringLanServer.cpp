#include "MonitoringLanServer.h"
#include "MonitoringNetServer.h"
#include "MonitorProtocol.h"
#include "../../00 lib_jy/Logger.h"
#include <iostream>

MonitoringLanServer::MonitoringLanServer(const char* systemFile, const char* server, NetServer* localServer) : NetServer(systemFile, server), localServer(localServer){
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
	p_dbConnector = new DBConnector(dbAddr, port, loginID, password, schema, loggingTime);

	dbEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	dbThread = std::thread([this] {DBWriteFunc(); });
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
			if (p_user->isLogin == false) {
				Disconnect(p_user->sessionID);
				LOG("MonitoringServer", LOG_LEVEL_WARN, "Disconnect // NOT LOGIN");
				return;
			}

			// * MonitoringNetSever로 모니터링 툴에게 모니터링 데이터 송신
			((MonitoringNetServer*)localServer)->BroadcastMonitoringData(p_user->serverNo, dataType, dataValue, timeStamp);
			SaveMonitoringData(p_user->serverNo, dataType, dataValue, timeStamp);
			return;
		}
	}
}

void MonitoringLanServer::DBWriteFunc(){
	for (;;) {
		WaitForSingleObject(dbEvent, INFINITE);
		for (;;) {
			if (dbJobQ.GetUseCount() < 1) break;
			DBJob* p_job;
			dbJobQ.Dequeue(&p_job);

			// DB Write
			p_dbConnector->Query("INSERT INTO logdb.monitorlog (serverno, type, avr, min, max) VALUES (%d, %d, %d, %d, %d)", p_job->serverNo, p_job->dataType, p_job->avr, p_job->min, p_job->max);
			dbJobPool.Free(p_job);
		}
	}
}

void MonitoringLanServer::SaveMonitoringData(int serverNo, int dataType, int data, int timeStamp){
	auto monitoringMapIter = serverMonitoredDataMap.find(serverNo);

	//////////////////////////////
	// 모니터링 데이터 save
	//////////////////////////////

	// 데이터가 최초 저장되는 서버
	if (monitoringMapIter == serverMonitoredDataMap.end()) {
		auto p_dataMap = new MonitoredDataMap;
		p_dataMap->insert({ dataType, new MonitoredData(data, timeStamp) });
		serverMonitoredDataMap.insert({ serverNo, p_dataMap });
		return;
	}

	MonitoredDataMap* p_dataMap = monitoringMapIter->second;
	auto MonitoredDataIter = p_dataMap->find(dataType);
	// 최초 저장되는 데이터 항목
	if (MonitoredDataIter == p_dataMap->end()) {
		p_dataMap->insert({ dataType, new MonitoredData(data, timeStamp) });
		return;
	}

	// 데이터 추가
	MonitoredData* p_data = MonitoredDataIter->second;
	p_data->AddData(data);

	// DB Write (10분 주기)
	if (p_data->lastWriteTime + 600 <= timeStamp) {
		DBJobQueuing(serverNo, dataType, p_data);
		p_data->Init(timeStamp);
	}
}

void MonitoringLanServer::DBJobQueuing(int serverNo, int dataType, MonitoredData* p_data) {
	auto p_job = dbJobPool.Alloc();
	p_job->serverNo = serverNo;
	p_job->dataType = dataType;
	p_job->avr = ((DWORD)p_data->sum / (DWORD)p_data->count);
	p_job->min = p_data->min;
	p_job->max = p_data->max;

	dbJobQ.Enqueue(p_job);
	SetEvent(dbEvent);
}