#pragma once
#include <windows.h>

class ThreadCpuMonitor {
public:
	ThreadCpuMonitor();

private:
	// core 사용 백분률
	float coreTotal = 0;
	float coreUser = 0;
	float coreKernel = 0;
	// 이전 업데이트 시간
	ULARGE_INTEGER prevTime = { 0, };
	// 누적 core 사용 시간 (이전 업데이트 기준)
	ULARGE_INTEGER prevUserTime = { 0, };
	ULARGE_INTEGER prevKernelTime = { 0, };

	// delta
	ULONGLONG deltaTime = 0;
	ULONGLONG deltaUserTime = 0;
	ULONGLONG deltaKernelTime = 0;
	ULONGLONG deltaTotalTime = 0;

public:
	void UpdateCpuUsage();
	void Init();

public:
	// Getter
	float GetTotalCpuUsage();
	float GetUserCpuUsage();
	float GetKernelCpuUsage();
	ULONGLONG GetDeltaTime();
	ULONGLONG GetDeltaUserTime();
	ULONGLONG GetDeltaKernelTime();
	ULONGLONG GetDeltaTotalTime();
};