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

	// ī���� ���
	PDH_HCOUNTER h_userMemory;		// ���μ��� �����Ҵ� �޸�
	PDH_HCOUNTER h_processNonMemory;// ���μ��� �������� �޸�
	PDH_HCOUNTER h_availMemory;		// �ý��ۿ��� �Ҵ簡���� �޸�
	PDH_HCOUNTER h_systemNonMemory;	// �ý��� �������� �޸�
	Ethernet ethernet[MAX_ETHERNET];// �̴��� ������ ��/���ŷ�

	// ī���� ������
	DWORD64 userMemory;
	DWORD64 processNonMemory;
	DWORD64 availMemory;
	DWORD64 systemNonMemory;
	DWORD64 recvBytes = 0;
	DWORD64 sendBytes = 0;

public:
	void Update();

public:
	DWORD64 GetUserMem();
	DWORD64 GetProcessNonMem();
	DWORD64 GetAvailMem();
	DWORD64 GetSysNonMem();
	DWORD64 GetRecvBytes();
	DWORD64 GetSendBytes();
};