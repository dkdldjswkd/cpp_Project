#include <stdio.h>
#include <Pdh.h>
#pragma comment(lib,"Pdh.lib")

int main(void) {
	// PDH ���� �ڵ� ����
	PDH_HQUERY cpuQuery;
	PdhOpenQuery(NULL, NULL, &cpuQuery);

	// PDH ���ҽ� ī���� ���� (������ ������ �̸� ������ ����)
	PDH_HCOUNTER cpuTotal;
	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);

	// ù ����
	PdhCollectQueryData(cpuQuery);

	while (true) {
		Sleep(1000);

		// 1�ʸ��� ����
		PdhCollectQueryData(cpuQuery);

		// ���� ������ ����
		PDH_FMT_COUNTERVALUE counterVal;
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

		// ���� ������ ���
		wprintf(L"CPU Usage : %f%%\n", counterVal.doubleValue);
	}

	return 0;
}

// \Processor(*)\% C1 Time