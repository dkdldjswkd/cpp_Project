#include "CpuUsage.h"

// Ȯ�� ���μ��� �ڵ�
CpuUsage::CpuUsage(HANDLE hProcess) {
	if (hProcess == INVALID_HANDLE_VALUE) {
		h_process = GetCurrentProcess();
	}

	// ���μ��� ����� == (���ð� / ���ھ� ��)
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	NumOfCore = SystemInfo.dwNumberOfProcessors; // �� �ھ� ����
	coreTotal = 0;
	coreUser = 0;
	coreKernel = 0;
	processTotal = 0;
	processUser = 0;
	processKernel = 0;
	prevKernelTime.QuadPart = 0;
	prevUserTime.QuadPart = 0;
	prevIdleTime.QuadPart = 0;
	prevUserTime.QuadPart = 0;
	prevKernelTime.QuadPart = 0;
	prevTime.QuadPart = 0;
	UpdateCpuTime();
}

// CPU ���� ���� (500 ~ 1000 ms �ֱ� ȣ��)
void CpuUsage::UpdateCpuTime() {
	ULONGLONG deltaTime;
	ULARGE_INTEGER curIdleTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	//---------------------------------------------------------
	// �ý��� ��ü �ھ� ���� ���
	//---------------------------------------------------------

	// ���� �ھ� ��� �ð� (Ŀ��, ����, idle)
	if (GetSystemTimes((PFILETIME)&curIdleTime, (PFILETIME)&curkernelTime, (PFILETIME)&curUserTime) == false) {
		return;
	}

	// ��Ÿ �ھ� ��� �ð� (���� - ������ ȣ��)
	ULONGLONG deltaIdleTime = curIdleTime.QuadPart - prevIdleTime.QuadPart;
	ULONGLONG deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	ULONGLONG deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	ULONGLONG Total = deltaKernelTime + deltaUserTime;

	if (Total == 0) {
		coreUser = 0.0f;
		coreKernel = 0.0f;
		coreTotal = 0.0f;
	}
	else {
		// �ھ� ��� ��з� ���
		coreTotal = (float)((double)(Total - deltaIdleTime) / Total * 100.0f);
		coreUser = (float)((double)deltaUserTime / Total * 100.0f);
		coreKernel = (float)((double)(deltaKernelTime - deltaIdleTime) / Total * 100.0f);
	}

	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
	prevIdleTime = curIdleTime;

	//---------------------------------------------------------
	// ��� ���μ��� �ھ� ���� ���
	//---------------------------------------------------------

	ULARGE_INTEGER None; // �̻�� ����
	ULARGE_INTEGER NowTime;

	// ���� �ð�
	GetSystemTimeAsFileTime((LPFILETIME)&NowTime);
	// �ش� ���μ��� ������� �ھ� ����
	GetProcessTimes(h_process, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&curkernelTime, (LPFILETIME)&curUserTime);

	// ��Ÿ �ھ� ��� �ð� (�ش� ���μ���������)
	deltaTime = NowTime.QuadPart - prevTime.QuadPart;
	deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	Total = deltaKernelTime + deltaUserTime;

	// �ھ� ��� �ð� ��з�
	processTotal = (float)(Total / (double)NumOfCore / (double)deltaTime * 100.0f);
	processKernel = (float)(deltaKernelTime / (double)NumOfCore / (double)deltaTime * 100.0f);
	processUser = (float)(deltaUserTime / (double)NumOfCore / (double)deltaTime * 100.0f);

	prevTime = NowTime;
	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
}

float CpuUsage::ProcessorTotal(void) {
	return coreTotal;
}

float CpuUsage::ProcessorUser(void) {
	return coreUser;
}

float CpuUsage::ProcessorKernel(void) {
	return coreKernel;
}

float CpuUsage::ProcessTotal(void) {
	return processTotal;
}

float CpuUsage::ProcessUser(void) {
	return processUser;
}

float CpuUsage::ProcessKernel(void) {
	return processKernel;
}