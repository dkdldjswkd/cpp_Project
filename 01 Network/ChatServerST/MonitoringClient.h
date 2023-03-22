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
	int serverNo;
	std::thread updateThread;
	bool isConnect = false;

	// Control Server
	NetServer* localServer;

	// ����͸�
	time_t lastUpdateTime;
	ProcessCpuMonitor  ProcessMonitor;
	MachineCpuMonitor  machineMonitor;
	PerformanceCounter perfCounter;

public:
	// ä�ü��� ����͸� ������
	int cpuUsageChat = 0;
	int usingMemoryMbChat = 0;
	int packetCount = 0;
	int sessionCount = 0;
	int userCount = 0;
	int updateTPS = 0;
	int jobCount = 0;

	// �ӽ� ����͸� ������
	int cpuUsageMachine = 0;
	int usingNonMemoryMbMachine = 0;
	int recvKbytes = 0;
	int sendKbytes = 0;
	int availMemMb = 0;

private:
	// Lib Callback
	void OnConnect();
	void OnRecv(PacketBuffer* contents_packet);
	void OnDisconnect();

private:
	void ReportToMonitoringServer();
	void UpdateData();
};