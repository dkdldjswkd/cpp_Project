#pragma once
#include <windows.h>

class ThreadCpuMonitor {
public:
	ThreadCpuMonitor();

private:
	// core ��� ��з�
	float coreTotal = 0;
	float coreUser = 0;
	float coreKernel = 0;
	// ���� ������Ʈ �ð�
	ULARGE_INTEGER prevTime = { 0, };
	// ���� core ��� �ð� (���� ������Ʈ ����)
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