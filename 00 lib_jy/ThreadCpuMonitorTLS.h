#pragma once
#include "TLSTemplate.h"

class ThreadCpuMonitor;
class ThreadCpuMonitorTLS : public TLSTemplate<ThreadCpuMonitor> {
public:
	ThreadCpuMonitorTLS();
	~ThreadCpuMonitorTLS();

public:
	// DataUpdate
	void UpdateCpuUsage();

	// Getter
	float GetTotalCpuUsage();
	float GetUserCpuUsage();
	float GetKernelCpuUsage();
};