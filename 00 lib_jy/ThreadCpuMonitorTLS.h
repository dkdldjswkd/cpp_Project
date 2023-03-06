#pragma once

class ThreadCpuMonitor;
class ThreadCpuMonitorTLS {
public:
	ThreadCpuMonitorTLS();
	~ThreadCpuMonitorTLS();

private:
	const DWORD tlsIndex;

private:
	ThreadCpuMonitor* Get();

public:
	void UpdateCpuUsage();

public:
	// Getter
	float GetTotalCpuUsage();
	float GetUserCpuUsage();
	float GetKernelCpuUsage();
};