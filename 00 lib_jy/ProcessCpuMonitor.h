#pragma once
#include <windows.h>

class ProcessCpuMonitor {
public:
	ProcessCpuMonitor(HANDLE hProcess = INVALID_HANDLE_VALUE); // 확인대상 프로세스 핸들

private:
	HANDLE h_process;
	int	NumOfCore;

	// core 사용 백분률
	float coreTotal = 0;
	float coreUser = 0;
	float coreKernel = 0;
	// 이전 업데이트 시간
	ULARGE_INTEGER prevTime = { 0, };
	// 누적 core 사용 시간 (이전 업데이트 기준)
	ULARGE_INTEGER prevUserTime = { 0, };
	ULARGE_INTEGER prevKernelTime = { 0, };

public:
	void UpdateCpuUsage();

public:
	// Getter
	float GetTotalCpuUsage();
	float GetUserCpuUsage();
	float GetKernelCpuUsage();
};