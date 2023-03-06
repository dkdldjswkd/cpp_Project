#pragma once
#include "../NetworkLib/NetClient.h"
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/ProcessCpuMonitor.h"
#include "../../00 lib_jy/MachineCpuMonitor.h"
#include "../../00 lib_jy/PerformanceCounter.h"
#include <thread>

class MonitoringClient : public NetClient {
public:
	MonitoringClient(const char* systemFile, const char* client, NetServer* localServer);
	~MonitoringClient();

private:
	std::thread updateThread;
	int serverNo;
	NetServer* localServer;
	bool isConnect = false;

	// 모니터링
	time_t lastUpdateTime;
	ProcessCpuMonitor  ProcessMonitor;
	MachineCpuMonitor  machineMonitor;
	PerformanceCounter perfCounter;

public:
	// 채팅서버 모니터링 데이터
	int cpuUsage_chattingServer = 0;
	int usingMemoryMB_chattingServer = 0;
	int packetCount_chattingServer = 0;
	int sessionCount_chattServer = 0;
	int userCount_chattServer = 0;
	int updateTPS_chattServer = 0;
	int jobCount_chattServe = 0;

	// 머신 모니터링 데이터
	int cpuUsage_machine = 0;
	int usingNonMemoryMB_machine = 0;
	int recvKbytes_machine = 0;
	int sendKbytes_machine = 0;
	int availMemMB_machine = 0;

private:
	void OnConnect();
	void OnRecv(PacketBuffer* contents_packet);
	void OnDisconnect();

private:
	void Report_to_monitoringServer();
	void UpdateData();
};