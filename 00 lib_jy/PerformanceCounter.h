#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <Pdh.h>

class PerformanceCounter{
public:
	PerformanceCounter();
	~PerformanceCounter();

private:
	struct Ethernet {
		bool isUse = 0;
		WCHAR name[128] = { 0, };
		PDH_HCOUNTER h_recvBytes = 0;
		PDH_HCOUNTER h_sendBytes = 0;
	};
#define MAX_ETHERNET 10

private:
	PDH_HQUERY h_pdh_query;

	// 카운터 목록
	PDH_HCOUNTER h_userMemory;		// 프로세스 유저할당 메모리
	PDH_HCOUNTER h_processNonMemory;// 프로세스 논페이지 메모리
	PDH_HCOUNTER h_availMemory;		// 시스템에서 할당가능한 메모리
	PDH_HCOUNTER h_systemNonMemory;	// 시스템 논페이지 메모리
	Ethernet ethernet[MAX_ETHERNET];// 이더넷 데이터 송/수신량

	// 카운터 데이터
	DWORD64 userMemory;
	DWORD64 processNonMemory;
	DWORD64 availMemory;
	DWORD64 systemNonMemory;
	DWORD64 recvBytes = 0;
	DWORD64 sendBytes = 0;

public:
	void Update();

public:
	DWORD64 GetUserMemB();
	DWORD64 GetProcessNonMemB();
	DWORD64 GetAvailMemMB();
	DWORD64 GetSysNonMemB();
	DWORD64 GetRecvBytes();
	DWORD64 GetSendBytes();
};