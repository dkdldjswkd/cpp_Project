#pragma once
#include <windows.h>

class CpuUsage {
public:
	CpuUsage(HANDLE hProcess = INVALID_HANDLE_VALUE); // 확인대상 프로세스 핸들

private:
	HANDLE h_process;
	int	NumOfCore;

	// core 사용 백분률
	float coreTotal;
	float coreUser;
	float coreKernel;
	// core 누적 사용률
	ULARGE_INTEGER prevKernelTime; // (Kernel + Idle)
	ULARGE_INTEGER prevUserTime;
	ULARGE_INTEGER prevIdleTime;

	// core 사용 백분률 (해당 프로세스에서)
	float processTotal;
	float processUser;
	float processKernel;
	// core 누적 사용률 (해당 프로세스에서)
	ULARGE_INTEGER prevKernelTime;
	ULARGE_INTEGER prevUserTime;
	// 마지막 Update 시간
	ULARGE_INTEGER prevTime;

public:
	void UpdateCpuTime(void);

public:
	// Getter
	float ProcessorTotal(void);
	float ProcessorUser(void);
	float ProcessorKernel(void);
	float ProcessTotal(void);
	float ProcessUser(void);
	float ProcessKernel(void);
};

