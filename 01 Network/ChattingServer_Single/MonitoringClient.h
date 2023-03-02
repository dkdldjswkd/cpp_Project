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
	ProcessCpuMonitor ProcessMonitor;
	MachineCpuMonitor machineMonitor;
	PerformanceCounter perfCounter;
	NetServer* localServer;

private:
	void OnConnect();
	void OnRecv(PacketBuffer* contents_packet);
	void OnDisconnect();

private:
	void UpdateFunc();
};