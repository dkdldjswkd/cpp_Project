#pragma once
#include <windows.h>

class ProcessCpuMonitor {
public:
	ProcessCpuMonitor(HANDLE hProcess = INVALID_HANDLE_VALUE); // Ȯ�δ�� ���μ��� �ڵ�

private:
	HANDLE h_process;
	int	NumOfCore;

	// core ��� ��з�
	float coreTotal = 0;
	float coreUser = 0;
	float coreKernel = 0;
	// ���� ������Ʈ �ð�
	ULARGE_INTEGER prevTime = { 0, };
	// ���� core ��� �ð� (���� ������Ʈ ����)
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