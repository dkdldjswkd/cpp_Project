#include "PerformanceCounter.h"
#include <PdhMsg.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib,"Pdh.lib")
using namespace std;

PerformanceCounter::PerformanceCounter(){
	// ���� ���μ��� �̸� ���
	TCHAR processName[MAX_PATH];
	TCHAR filePath[MAX_PATH];
	GetModuleFileNameW(GetModuleHandleW(NULL), filePath, MAX_PATH);
	_tsplitpath_s(filePath, NULL, 0, NULL, 0, processName, MAX_PATH, NULL, 0);

	// PDH ���� �ڵ� ����
	PdhOpenQuery(NULL, NULL, &h_pdh_query);

	// PDH �޸� �׸� ���� ����
	TCHAR query_user_mem[MAX_PATH];
	TCHAR query_process_non_mem[MAX_PATH];
	swprintf_s(query_user_mem, MAX_PATH, L"\\Process(%s)\\Private Bytes", processName);
	swprintf_s(query_process_non_mem, MAX_PATH, L"\\Process(%s)\\Pool Nonpaged Bytes", processName);

	// PDH �޸��׸� ī���� ���
	PdhAddCounter(h_pdh_query, query_user_mem,					NULL, &h_userMemory);		// ���μ��� �����Ҵ� �޸�
	PdhAddCounter(h_pdh_query, query_process_non_mem,			NULL, &h_processNonMemory);	// ���μ��� �������� �޸�
	PdhAddCounter(h_pdh_query, L"\\Memory\\Available MBytes",	NULL, &h_availMemory);		// �ý��ۿ��� �Ҵ簡���� �޸�
	PdhAddCounter(h_pdh_query, L"\\Memory\\Pool Nonpaged Bytes",NULL, &h_systemNonMemory);	// �ý��� �������� �޸�

	// �̴��� ī�� �̸� ���
	DWORD counterSize = 0, interfaceSize = 0;
	PdhEnumObjectItems(NULL, NULL, L"Network Interface", NULL, &counterSize, NULL, &interfaceSize, PERF_DETAIL_WIZARD, 0);
	WCHAR* counters = new WCHAR[counterSize];
	WCHAR* interfaces = new WCHAR[interfaceSize];
	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", counters, &counterSize, interfaces, &interfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS) {
		delete[] counters;
		delete[] interfaces;
		return;
	}

	// PDH ��Ʈ��ũ �׸� ���� ���� �� ī���� ���
	WCHAR* offset = interfaces;
	WCHAR query_network[MAX_PATH];
	for (int i = 0;; i++) {
		// �̴��� ����ü �ʱ�ȭ
		ethernet[i].isUse = true;
		wcscpy_s(ethernet[i].name, offset);
		// Recv ���ŷ� ���� ���� �� ī���� ���
		swprintf_s(query_network, MAX_PATH, L"\\Network Interface(%s)\\Bytes Received/sec", offset);
		PdhAddCounter(h_pdh_query, query_network, NULL, &ethernet[i].h_recvBytes);
		// Send �۽ŷ� ���� ���� �� ī���� ���
		swprintf_s(query_network, MAX_PATH, L"\\Network Interface(%s)\\Bytes Sent/sec", offset);
		PdhAddCounter(h_pdh_query, query_network, NULL, &ethernet[i].h_sendBytes);
		// ���� ���� ����
		offset += wcslen(offset) + 1; // ���� ���ڿ�
		if (*offset == L'\0' || MAX_ETHERNET <= i) break;
	}
	delete[] counters;
	delete[] interfaces;

	// ������ ����
	Update();
}	
PerformanceCounter::~PerformanceCounter() {}

void PerformanceCounter::Update() {
	PDH_FMT_COUNTERVALUE counterVal;

	// PDH ������ ����
	PdhCollectQueryData(h_pdh_query);

	// �޸� ī���� ������ �ֽ�ȭ
	PdhGetFormattedCounterValue(h_userMemory, PDH_FMT_LARGE, NULL, &counterVal);
	userMemory = counterVal.largeValue;
	PdhGetFormattedCounterValue(h_processNonMemory, PDH_FMT_LARGE, NULL, &counterVal);
	processNonMemory = counterVal.largeValue;
	PdhGetFormattedCounterValue(h_availMemory, PDH_FMT_LARGE, NULL, &counterVal);
	availMemory = counterVal.largeValue;
	PdhGetFormattedCounterValue(h_systemNonMemory, PDH_FMT_LARGE, NULL, &counterVal);
	systemNonMemory = counterVal.largeValue;

	// ��Ʈ��ũ ī���� ������ �ֽ�ȭ
	recvBytes = 0;
	sendBytes = 0;
	for (int i = 0; i < MAX_ETHERNET && ethernet[i].isUse; i++) {
		PdhGetFormattedCounterValue(ethernet[i].h_recvBytes, PDH_FMT_LARGE, NULL, &counterVal);
		recvBytes += counterVal.largeValue;
		PdhGetFormattedCounterValue(ethernet[i].h_sendBytes, PDH_FMT_LARGE, NULL, &counterVal);
		sendBytes += counterVal.largeValue;
	}
}

DWORD64 PerformanceCounter::GetUserMemB(){
	return userMemory;
}

DWORD64 PerformanceCounter::GetProcessNonMemB(){
	return processNonMemory;
}

DWORD64 PerformanceCounter::GetAvailMemMB(){
	return availMemory;
}

DWORD64 PerformanceCounter::GetSysNonMemB(){
	return systemNonMemory;
}

DWORD64 PerformanceCounter::GetRecvBytes(){
	return recvBytes;
}

DWORD64 PerformanceCounter::GetSendBytes(){
	return sendBytes;
}

/*
PDH �ʼ����� ����͸� �׸�
	* ���μ��� �����Ҵ� �޸�
	* ���μ��� �������� �޸�
	* ��밡�� �޸�
	* ���������� �޸�
	+ ��Ʈ��ũ ��뷮
		�� �ָ���

�޸�
	���μ���
		���μ��� �����Ҵ� �޸�
			"\\Process(NAME)\\Private Bytes"
				�̰��� �츮�� ����� ���� ���� �޸� ��뷮
				���μ��� ����޸� / Ŀ�θ޸� ����
		���μ��� �������� �޸�
			"\\Process(NAME)\\Pool Nonpaged Bytes"
		���μ��� ����޸� ���
			"\\Process(NAME)\\Virtual Bytes"
				�̴� ����޸� ���̺��� �ּ� �뷮���� ���� ���޸𸮿ʹ� ����
		���μ��� �۾� �޸�
			"\\Process(NAME)\\Working Set"
				���� ���� �޸𸮿� ���Ǵ� ũ���� �� �Ҵ� �뷮�� �ƴ� �� ����.
	�ý��� ��ü
		��밡�� �޸�
			L"\\Memory\\Available MBytes"
		���������� �޸�
			L"\\Memory\\Pool Nonpaged Bytes"

CPU ����
	CPU ��ü ����
		"\\Processor(_Total)\\% Processor Time"
	CPU �ھ� ����
		"\\Processor(0)\\% Processor Time"
		"\\Processor(1)\\% Processor Time"
		"\\Processor(2)\\% Processor Time"
		"\\Processor(3)\\% Processor Time"
	���μ��� CPU ���� ����
		"\\Process(NAME)\\% User Time"
	���μ��� CPU ��ü ����
		"\\Process(NAME)\\% Processor Time"

�ý��� ���ҽ� (�ڵ�, ������)
	���μ��� �ڵ� ��
		"\\Process(NAME)\\Handle Count"
	���μ��� ������ ��
		"\\Process(NAME)\\Thread Count"
*/