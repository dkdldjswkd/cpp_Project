#include "ThreadCpuMonitor.h"
#include <iostream>

// 확인 프로세스 핸들
ThreadCpuMonitor::ThreadCpuMonitor() {
	UpdateCpuUsage();
}

// CPU 사용률 갱신 (500 ~ 1000 ms 주기 호출)
void ThreadCpuMonitor::UpdateCpuUsage() {
	ULARGE_INTEGER none; // 미사용
	ULARGE_INTEGER curTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	// 현재시간 & 누적 코어 사용시간
	GetSystemTimeAsFileTime((LPFILETIME)&curTime);
	GetThreadTimes(GetCurrentThread(), (FILETIME*)&none, (FILETIME*)&none, (FILETIME*)&curkernelTime, (FILETIME*)&curUserTime);

	// 스레드 사용시간
	deltaTime = (curTime.QuadPart - prevTime.QuadPart);
	deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	deltaTotalTime = deltaKernelTime + deltaUserTime;

	// 코어 사용 시간 백분률
	coreTotal = (float)(deltaTotalTime / (double)deltaTime * 100.0f);
	coreUser = (float)((deltaUserTime / (double)deltaTime * 100.0f) / coreTotal) * 100;
	coreKernel = (float)((deltaKernelTime / (double)deltaTime * 100.0f) / coreTotal) * 100;

	prevTime = curTime;
	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
}

void ThreadCpuMonitor::Init() {
	ULARGE_INTEGER none; // 미사용
	GetSystemTimeAsFileTime((LPFILETIME)&prevTime);
	GetThreadTimes(GetCurrentThread(), (FILETIME*)&none, (FILETIME*)&none, (FILETIME*)&prevKernelTime, (FILETIME*)&prevUserTime);
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

ULONGLONG ThreadCpuMonitor::GetDeltaTime(){
	return deltaTime;
}

ULONGLONG ThreadCpuMonitor::GetDeltaUserTime(){
	return deltaUserTime ;
}

ULONGLONG ThreadCpuMonitor::GetDeltaKernelTime(){
	return deltaKernelTime ;
}

ULONGLONG ThreadCpuMonitor::GetDeltaTotalTime(){
	return deltaTotalTime;
}
