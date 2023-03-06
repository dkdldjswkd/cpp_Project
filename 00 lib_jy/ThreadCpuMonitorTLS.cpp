#include "ThreadCpuMonitor.h"
#include "ThreadCpuMonitorTLS.h"

ThreadCpuMonitorTLS::ThreadCpuMonitorTLS() : TLSTemplate<ThreadCpuMonitor>()  {
}

ThreadCpuMonitorTLS::~ThreadCpuMonitorTLS() {
}

void ThreadCpuMonitorTLS::UpdateCpuUsage(){
	Get()->UpdateCpuUsage();
}

float ThreadCpuMonitorTLS::GetTotalCpuUsage(){
	return Get()->GetTotalCpuUsage();
}

float ThreadCpuMonitorTLS::GetUserCpuUsage(){
	return Get()->GetUserCpuUsage();
}

float ThreadCpuMonitorTLS::GetKernelCpuUsage(){
	return Get()->GetKernelCpuUsage();
}