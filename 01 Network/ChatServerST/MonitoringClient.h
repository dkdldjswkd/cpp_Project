#pragma once
#include "../NetworkLib/NetClient.h"
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/ProcessCpuMonitor.h"
#include "../../00 lib_jy/MachineCpuMonitor.h"
#include "../../00 lib_jy/PerformanceCounter.h"
#include <thread>

class MonitoringClient : public NetClient {
public:
	MonitoringClient(const char* systemFile, const char* client, ChatServerST* localServer);
	~MonitoringClient();

private:
	int serverNo;
	std::thread updateThread;
	bool updateRun = true;
	bool isConnect = false;

	// Control Server
	ChatServerST* p_chatServer;

	// 모니터링
	time_t lastUpdateTime;
	ProcessCpuMonitor  ProcessMonitor;
	MachineCpuMonitor  machineMonitor;
	PerformanceCounter perfCounter;
	// 채팅서버 모니터링 데이터
	int processCpuUsage = 0;
	int processUsingMemMb = 0;
	// 머신 모니터링 데이터
	int machineCpuUsage = 0;
	int machineUsingNonMemMb = 0;
	int machineRecvKbytes = 0;
	int machineSendKbytes = 0;
	int machineAvailMemMb = 0;

private:
	// Lib Callback
	void OnConnect();
	void OnRecv(PacketBuffer* contents_packet);
	void OnDisconnect();
	void OnClientStop();

	// Monitor & Report (To.모니터링 서버)
	void UpdateData();
	void ReportToMonitoringServer();

public:
	// Getter
	int GetProcessCpuUsage();
	int GetProcessUsingMemMb();
	int GetMachineCpuUsage();
	int GetMachineUsingNonMemMb();
	int GetMachineRecvKbytes();
	int GetMachineSendKbytes();
	int GetMachineAvailMemMb();
};

inline int MonitoringClient::GetProcessCpuUsage() {
	return processCpuUsage;
}

inline int MonitoringClient::GetProcessUsingMemMb() {
	return processUsingMemMb;
}

inline int MonitoringClient::GetMachineCpuUsage() {
	return machineCpuUsage;
}

inline int MonitoringClient::GetMachineUsingNonMemMb() {
	return machineUsingNonMemMb;
}

inline int MonitoringClient::GetMachineRecvKbytes() {
	return machineRecvKbytes;
}

inline int MonitoringClient::GetMachineSendKbytes() {
	return machineSendKbytes;
}

inline int MonitoringClient::GetMachineAvailMemMb() {
	return machineAvailMemMb;
}