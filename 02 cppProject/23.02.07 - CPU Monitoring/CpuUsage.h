#pragma once
#include <windows.h>

class CpuUsage {
public:
	CpuUsage(HANDLE hProcess = INVALID_HANDLE_VALUE); // Ȯ�δ�� ���μ��� �ڵ�

private:
	HANDLE h_process;
	int	NumOfCore;

	// core ��� ��з�
	float coreTotal;
	float coreUser;
	float coreKernel;
	// core ���� ����
	ULARGE_INTEGER prevKernelTime; // (Kernel + Idle)
	ULARGE_INTEGER prevUserTime;
	ULARGE_INTEGER prevIdleTime;

	// core ��� ��з� (�ش� ���μ�������)
	float processTotal;
	float processUser;
	float processKernel;
	// core ���� ���� (�ش� ���μ�������)
	ULARGE_INTEGER prevKernelTime;
	ULARGE_INTEGER prevUserTime;
	// ������ Update �ð�
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

