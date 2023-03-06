#include "ThreadCpuMonitor.h"
#include <iostream>

// Ȯ�� ���μ��� �ڵ�
ThreadCpuMonitor::ThreadCpuMonitor() {
	UpdateCpuUsage();
}

// CPU ���� ���� (500 ~ 1000 ms �ֱ� ȣ��)
void ThreadCpuMonitor::UpdateCpuUsage() {
	ULARGE_INTEGER none; // �̻��
	ULARGE_INTEGER curTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	// ����ð� & ���� �ھ� ���ð�
	GetSystemTimeAsFileTime((LPFILETIME)&curTime);
	GetThreadTimes(GetCurrentThread(), (FILETIME*)&none, (FILETIME*)&none, (FILETIME*)&curkernelTime, (FILETIME*)&curUserTime);

	// ������ ���ð�
	ULONGLONG deltaTime = curTime.QuadPart - prevTime.QuadPart;
	ULONGLONG deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	ULONGLONG deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	ULONGLONG totalTime = deltaKernelTime + deltaUserTime;

	// �ھ� ��� �ð� ��з�
	coreTotal = (float)(totalTime / (double)deltaTime * 100.0f);
	coreUser = (float)((deltaUserTime / (double)deltaTime * 100.0f) / coreTotal) * 100;
	coreKernel = (float)((deltaKernelTime / (double)deltaTime * 100.0f) / coreTotal) * 100;

	prevTime = curTime;
	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
}

float ThreadCpuMonitor::GetTotalCpuUsage() {
	return coreTotal;
}

float ThreadCpuMonitor::GetUserCpuUsage() {
	return coreUser;
}

float ThreadCpuMonitor::GetKernelCpuUsage() {
	return coreKernel;
}