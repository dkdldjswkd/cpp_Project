#include "ThreadCpuMonitor.h"
#include "ThreadCpuMonitorTLS.h"

ThreadCpuMonitorTLS::ThreadCpuMonitorTLS() : tlsIndex(TlsAlloc()) {
}

ThreadCpuMonitorTLS::~ThreadCpuMonitorTLS() {
}

ThreadCpuMonitor* ThreadCpuMonitorTLS::Get() {
	ThreadCpuMonitor* p = (ThreadCpuMonitor*)TlsGetValue(tlsIndex);
	if (nullptr == p) {
		p = new ThreadCpuMonitor();
		TlsSetValue(tlsIndex, (LPVOID)p);
	}

	return p;
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