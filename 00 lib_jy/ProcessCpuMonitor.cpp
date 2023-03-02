#include "ProcessCpuMonitor.h"
#include <iostream>

// Ȯ�� ���μ��� �ڵ�
ProcessCpuMonitor::ProcessCpuMonitor(HANDLE hProcess){
	if (hProcess == INVALID_HANDLE_VALUE) {
		h_process = GetCurrentProcess();
	}

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	NumOfCore = SystemInfo.dwNumberOfProcessors; // �� �ھ� ����
	UpdateCpuUsage();
}

// CPU ���� ���� (500 ~ 1000 ms �ֱ� ȣ��)
void ProcessCpuMonitor::UpdateCpuUsage() {
	ULARGE_INTEGER none; // �̻��
	ULARGE_INTEGER curTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	// ����ð� & ���� �ھ� ���ð�
	GetSystemTimeAsFileTime((LPFILETIME)&curTime);
	GetProcessTimes(h_process, (LPFILETIME)&none, (LPFILETIME)&none, (LPFILETIME)&curkernelTime, (LPFILETIME)&curUserTime);

	// �ھ� ��� �ð� (�ش� ���μ�����)
	ULONGLONG deltaTime			= curTime.QuadPart		 - prevTime.QuadPart;
	ULONGLONG deltaUserTime		= curUserTime.QuadPart	 - prevUserTime.QuadPart;
	ULONGLONG deltaKernelTime	= curkernelTime.QuadPart - prevKernelTime.QuadPart;
	ULONGLONG totalTime = deltaKernelTime + deltaUserTime;

	// �ھ� ��� �ð� ��з�
	coreTotal = (float)(totalTime / (double)NumOfCore / (double)deltaTime * 100.0f);
	coreUser = (float)(deltaUserTime / (double)NumOfCore / (double)deltaTime * 100.0f);
	coreKernel = (float)(deltaKernelTime / (double)NumOfCore / (double)deltaTime * 100.0f);

	prevTime = curTime;
	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
}

float ProcessCpuMonitor::GetTotalCpuUsage() {
	return coreTotal;
}

float ProcessCpuMonitor::GetUserCpuUsage() {
	return coreUser;
}

float ProcessCpuMonitor::GetKernelCpuUsage() {
	return coreKernel;
}