#include "SegmentCpuMonitor.h"
#include <iostream>

// 확인 프로세스 핸들
SegmentCpuMonitor::SegmentCpuMonitor() {
	QueryPerformanceFrequency(&frequency);
}

void SegmentCpuMonitor::Begin() {
	QueryPerformanceCounter(&startTime);

	ULARGE_INTEGER none; // 미사용
	GetThreadTimes(GetCurrentThread(), (FILETIME*)&none, (FILETIME*)&none, (FILETIME*)&prevKernelTime, (FILETIME*)&prevUserTime);
}

double SegmentCpuMonitor::End() {
	QueryPerformanceCounter(&endTime);

	ULARGE_INTEGER none; // 미사용
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;
	GetThreadTimes(GetCurrentThread(), (FILETIME*)&none, (FILETIME*)&none, (FILETIME*)&curkernelTime, (FILETIME*)&curUserTime);

	deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	deltaTotalTime = deltaKernelTime + deltaUserTime;

	return timeUint * (double)(endTime.QuadPart - startTime.QuadPart) / (double)frequency.QuadPart;
}

void SegmentCpuMonitor::SetSec() {
	timeUint = 1;
}

void SegmentCpuMonitor::SetMilli() {
	timeUint = 1 * 1000;
}

void SegmentCpuMonitor::SetMicro() {
	timeUint = 1 * 1000 * 1000;
}

void SegmentCpuMonitor::SetNano() {
	timeUint = 1 * 1000 * 1000 * 1000;
}

//////////////////////////////
// Getter
//////////////////////////////

double SegmentCpuMonitor::GetUserTime(){
	return (double)deltaUserTime * (timeUint / 10000000);
}

double SegmentCpuMonitor::GetKernelTime(){
	return (double)deltaKernelTime * (timeUint / 10000000);
}

double SegmentCpuMonitor::GetTotalTime(){
	return (double)deltaTotalTime * (timeUint / 10000000);
}