#pragma once
#include <windows.h>

class MachineCpuMonitor {
public:
	MachineCpuMonitor(); // 확인대상 프로세스 핸들

private:
	int	NumOfCore;

	// core 사용 백분률
	float coreTotal = 0;
	float coreUser = 0;
	float coreKernel = 0;
	// 누적 core 사용 시간 (이전 업데이트 기준)
	ULARGE_INTEGER prevKernelTime = { 0, };
	ULARGE_INTEGER prevUserTime = { 0, };
	ULARGE_INTEGER prevIdleTime = { 0, };

public:
	void UpdateCpuUsage();

public:
	// Getter
	float GetTotalCpuUsage();
	float GetUserCpuUsage();
	float GetKernelCpuUsage();
};

