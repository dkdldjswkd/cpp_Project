#pragma once
#include <windows.h>

class MachineCpuMonitor {
public:
	MachineCpuMonitor(); // Ȯ�δ�� ���μ��� �ڵ�

private:
	int	NumOfCore;

	// core ��� ��з�
	float coreTotal = 0;
	float coreUser = 0;
	float coreKernel = 0;
	// ���� core ��� �ð� (���� ������Ʈ ����)
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

