#pragma once
#include "../NetworkLib/NetClient.h"
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/ProcessCpuMonitor.h"
#include "../../00 lib_jy/MachineCpuMonitor.h"
#include "../../00 lib_jy/PerformanceCounter.h"
#include <thread>

class ChatServerMTMonitorClient : public NetClient {
public:
	ChatServerMTMonitorClient(const char* systemFile, const char* client, ChatServerMT* localServer);
	~ChatServerMTMonitorClient();

private:
	int serverNo;
	std::thread updateThread;
	bool updateRun = true;
	bool isConnect = false;

	// Control Server
	ChatServerMT* p_chatServer;

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
	void OnRecv(PacketBuffer* contents_packet) {}
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

inline int ChatServerMTMonitorClient::GetProcessCpuUsage() {
	return processCpuUsage;
}

inline int ChatServerMTMonitorClient::GetProcessUsingMemMb() {
	return processUsingMemMb;
}

inline int ChatServerMTMonitorClient::GetMachineCpuUsage() {
	return machineCpuUsage;
}

inline int ChatServerMTMonitorClient::GetMachineUsingNonMemMb() {
	return machineUsingNonMemMb;
}

inline int ChatServerMTMonitorClient::GetMachineRecvKbytes() {
	return machineRecvKbytes;
}

inline int ChatServerMTMonitorClient::GetMachineSendKbytes() {
	return machineSendKbytes;
}

inline int ChatServerMTMonitorClient::GetMachineAvailMemMb() {
	return machineAvailMemMb;
}