#include "PerformanceCounter.h"
#include <PdhMsg.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib,"Pdh.lib")
using namespace std;

PerformanceCounter::PerformanceCounter(){
	// 현제 프로세스 이름 얻기
	TCHAR processName[MAX_PATH];
	TCHAR filePath[MAX_PATH];
	GetModuleFileNameW(GetModuleHandleW(NULL), filePath, MAX_PATH);
	_tsplitpath_s(filePath, NULL, 0, NULL, 0, processName, MAX_PATH, NULL, 0);

	// PDH 쿼리 핸들 생성
	PdhOpenQuery(NULL, NULL, &h_pdh_query);

	// PDH 메모리 항목 쿼리 생성
	TCHAR query_user_mem[MAX_PATH];
	TCHAR query_process_non_mem[MAX_PATH];
	swprintf_s(query_user_mem, MAX_PATH, L"\\Process(%s)\\Private Bytes", processName);
	swprintf_s(query_process_non_mem, MAX_PATH, L"\\Process(%s)\\Pool Nonpaged Bytes", processName);

	// PDH 메모리항목 카운터 등록
	PdhAddCounter(h_pdh_query, query_user_mem,					NULL, &h_userMemory);		// 프로세스 유저할당 메모리
	PdhAddCounter(h_pdh_query, query_process_non_mem,			NULL, &h_processNonMemory);	// 프로세스 논페이지 메모리
	PdhAddCounter(h_pdh_query, L"\\Memory\\Available MBytes",	NULL, &h_availMemory);		// 시스템에서 할당가능한 메모리
	PdhAddCounter(h_pdh_query, L"\\Memory\\Pool Nonpaged Bytes",NULL, &h_systemNonMemory);	// 시스템 논페이지 메모리

	// 이더넷 카드 이름 얻기
	DWORD counterSize = 0, interfaceSize = 0;
	PdhEnumObjectItems(NULL, NULL, L"Network Interface", NULL, &counterSize, NULL, &interfaceSize, PERF_DETAIL_WIZARD, 0);
	WCHAR* counters = new WCHAR[counterSize];
	WCHAR* interfaces = new WCHAR[interfaceSize];
	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", counters, &counterSize, interfaces, &interfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS) {
		delete[] counters;
		delete[] interfaces;
		return;
	}

	// PDH 네트워크 항목 쿼리 생성 및 카운터 등록
	WCHAR* offset = interfaces;
	WCHAR query_network[MAX_PATH];
	for (int i = 0;; i++) {
		// 이더넷 구조체 초기화
		ethernet[i].isUse = true;
		wcscpy_s(ethernet[i].name, offset);
		// Recv 수신량 쿼리 생성 및 카운터 등록
		swprintf_s(query_network, MAX_PATH, L"\\Network Interface(%s)\\Bytes Received/sec", offset);
		PdhAddCounter(h_pdh_query, query_network, NULL, &ethernet[i].h_recvBytes);
		// Send 송신량 쿼리 생성 및 카운터 등록
		swprintf_s(query_network, MAX_PATH, L"\\Network Interface(%s)\\Bytes Sent/sec", offset);
		PdhAddCounter(h_pdh_query, query_network, NULL, &ethernet[i].h_sendBytes);
		// 다음 루프 셋팅
		offset += wcslen(offset) + 1; // 다음 문자열
		if (*offset == L'\0' || MAX_ETHERNET <= i) break;
	}
	delete[] counters;
	delete[] interfaces;

	// 데이터 갱신
	Update();
}	
PerformanceCounter::~PerformanceCounter() {}

void PerformanceCounter::Update() {
	PDH_FMT_COUNTERVALUE counterVal;

	// PDH 데이터 갱신
	PdhCollectQueryData(h_pdh_query);

	// 메모리 카운터 데이터 최신화
	PdhGetFormattedCounterValue(h_userMemory, PDH_FMT_LARGE, NULL, &counterVal);
	userMemory = counterVal.largeValue;
	PdhGetFormattedCounterValue(h_processNonMemory, PDH_FMT_LARGE, NULL, &counterVal);
	processNonMemory = counterVal.largeValue;
	PdhGetFormattedCounterValue(h_availMemory, PDH_FMT_LARGE, NULL, &counterVal);
	availMemory = counterVal.largeValue;
	PdhGetFormattedCounterValue(h_systemNonMemory, PDH_FMT_LARGE, NULL, &counterVal);
	systemNonMemory = counterVal.largeValue;

	// 네트워크 카운터 데이터 최신화
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
PDH 필수적인 모니터링 항목
	* 프로세스 유저할당 메모리
	* 프로세스 논페이지 메모리
	* 사용가능 메모리
	* 논페이지드 메모리
	+ 네트워크 사용량
		좀 애매함

메모리
	프로세스
		프로세스 유저할당 메모리
			"\\Process(NAME)\\Private Bytes"
				이것이 우리가 사용한 실제 유저 메모리 사용량
				프로세스 공용메모리 / 커널메모리 제외
		프로세스 논페이지 메모리
			"\\Process(NAME)\\Pool Nonpaged Bytes"
		프로세스 가상메모리 사용
			"\\Process(NAME)\\Virtual Bytes"
				이는 가상메모리 테이블의 주소 용량으로 실제 사용메모리와는 무관
		프로세스 작업 메모리
			"\\Process(NAME)\\Working Set"
				현재 물리 메모리에 사용되는 크기일 뿐 할당 용량은 아닐 수 있음.
	시스템 전체
		사용가능 메모리
			L"\\Memory\\Available MBytes"
		논페이지드 메모리
			L"\\Memory\\Pool Nonpaged Bytes"

CPU 사용률
	CPU 전체 사용률
		"\\Processor(_Total)\\% Processor Time"
	CPU 코어 사용률
		"\\Processor(0)\\% Processor Time"
		"\\Processor(1)\\% Processor Time"
		"\\Processor(2)\\% Processor Time"
		"\\Processor(3)\\% Processor Time"
	프로세스 CPU 유저 사용률
		"\\Process(NAME)\\% User Time"
	프로세스 CPU 전체 사용률
		"\\Process(NAME)\\% Processor Time"

시스템 리소스 (핸들, 스레드)
	프로세스 핸들 수
		"\\Process(NAME)\\Handle Count"
	프로세스 스레드 수
		"\\Process(NAME)\\Thread Count"
*/