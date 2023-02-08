#include <Windows.h>
#include <pdh.h>
#include <strsafe.h>
#define df_PDH_ETHERNET_MAX 8

//--------------------------------------------------------------
// �̴��� �ϳ��� ���� Send,Recv PDH ���� ����.
//--------------------------------------------------------------
struct st_ETHERNET {
	bool		_bUse;
	WCHAR		_szName[128];

	PDH_HCOUNTER		_pdh_Counter_Network_RecvBytes;
	PDH_HCOUNTER		_pdh_Counter_Network_SendBytes;
};

st_ETHERNET _EthernetStruct[df_PDH_ETHERNET_MAX]; // ��ī�� �� PDH ����
double _pdh_value_Network_RecvBytes; // �� Recv Bytes ��� �̴����� Recv ��ġ �ջ�
double _pdh_value_Network_SendBytes; // �� Send Bytes ��� �̴����� Send ��ġ �ջ�

///////////////////////////////////////////////////////////////////////////////////////////////////////

int iCnt = 0;
bool bErr = false;
WCHAR* szCur = NULL;
WCHAR* szCounters = NULL;
WCHAR* szInterfaces = NULL;
DWORD dwCounterSize = 0, dwInterfaceSize = 0;
WCHAR szQuery[1024] = { 0, };

// PDH enum Object �� ����ϴ� ���.
// ��� �̴��� �̸��� �������� ���� ������� �̴���, �����̴��� ����� Ȯ�κҰ� ��.

int main() {
	//---------------------------------------------------------------------------------------
	// PdhEnumObjectItems �� ���ؼ� "NetworkInterface" �׸񿡼� ���� �� �ִ�
	// �����׸�(Counters) / �������̽� �׸�(Interfaces) �� ����. �׷��� �� ������ ���̸� �𸣱� ������
	// ���� ������ ���̸� �˱� ���ؼ� Out Buffer ���ڵ��� NULL �����ͷ� �־ ����� Ȯ��.
	//---------------------------------------------------------------------------------------
	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);

	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];

	//---------------------------------------------------------------------------------------
	// ������ �����Ҵ� �� �ٽ� ȣ��!
	//
	// szCounters �� szInterfaces ���ۿ��� �������� ���ڿ��� ������ ���´�. 2���� �迭�� �ƴϰ�,
	// �׳� NULL �����ͷ� ������ ���ڿ����� dwCounterSize, dwInterfaceSize ���̸�ŭ ������ �������.
	// �̸� ���ڿ� ������ ��� ������ Ȯ�� �ؾ� ��. aaa\0bbb\0ccc\0ddd �̵� ��
	//---------------------------------------------------------------------------------------
	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS) {
		delete[] szCounters;
		delete[] szInterfaces;
		return false;
	}

	iCnt = 0;
	szCur = szInterfaces;

	//---------------------------------------------------------
	// szInterfaces ���� ���ڿ� ������ �����鼭 , �̸��� ����޴´�.
	//---------------------------------------------------------
	PDH_HQUERY _pdh_Query = INVALID_HANDLE_VALUE; // ?? �� �ڵ�
	for (; *szCur != L'\0' && iCnt < df_PDH_ETHERNET_MAX; szCur += wcslen(szCur) + 1, iCnt++) {
		_EthernetStruct[iCnt]._bUse = true;
		_EthernetStruct[iCnt]._szName[0] = L'\0';
		wcscpy_s(_EthernetStruct[iCnt]._szName, szCur);
		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Received/sec", szCur);
		PdhAddCounter(_pdh_Query, szQuery, NULL, &_EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes);
		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Sent/sec", szCur);
		PdhAddCounter(_pdh_Query, szQuery, NULL, &_EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes);
	}

	///////////////////////////////////////////////////////////////////////////
	// ������ �������
	// _EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes
	// _EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes
	///////////////////////////////////////////////////////////////////////////

	//PDH ī���͸� �ٸ� PDH ī���Ϳ� ���� ������� ��� ���ָ� ��.
	//-----------------------------------------------------------------------------------------------
	// �̴��� ������ŭ ���鼭 �� ���� ����.
	//-----------------------------------------------------------------------------------------------
	PDH_STATUS Status;
	PDH_FMT_COUNTERVALUE CounterValue;
	for (int iCnt = 0; iCnt < df_PDH_ETHERNET_MAX; iCnt++) {
		if (_EthernetStruct[iCnt]._bUse) {
			Status = PdhGetFormattedCounterValue(_EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes, PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0) _pdh_value_Network_RecvBytes += CounterValue.doubleValue;
			Status = PdhGetFormattedCounterValue(_EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes, PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0) _pdh_value_Network_SendBytes += CounterValue.doubleValue;
		}
	}
}