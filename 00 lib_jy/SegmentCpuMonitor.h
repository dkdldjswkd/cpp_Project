#pragma once
#include <windows.h>

class SegmentCpuMonitor {
public:
	SegmentCpuMonitor();

private:
	// �ð� ����
	LARGE_INTEGER frequency;
	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;

	// ��ȯ ����
	double timeUint = 1;

private:
	ULARGE_INTEGER prevUserTime = { 0, };
	ULARGE_INTEGER prevKernelTime = { 0, };

	ULONGLONG deltaUserTime = 0;
	ULONGLONG deltaKernelTime = 0;
	ULONGLONG deltaTotalTime = 0;

public:
	void Begin();
	double End();

public:
	void SetSec();
	void SetMilli();
	void SetMicro();
	void SetNano();

public:
	// Getter
	double GetUserTime();
	double GetKernelTime();
	double GetTotalTime();
};