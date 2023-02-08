#include <stdio.h>
#include <Pdh.h>
#pragma comment(lib,"Pdh.lib")

int main(void) {
	// PDH 쿼리 핸들 생성
	PDH_HQUERY cpuQuery;
	PdhOpenQuery(NULL, NULL, &cpuQuery);

	// PDH 리소스 카운터 생성 (여러개 수집시 이를 여러개 생성)
	PDH_HCOUNTER cpuTotal;
	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);

	// 첫 갱신
	PdhCollectQueryData(cpuQuery);

	while (true) {
		Sleep(1000);

		// 1초마다 갱신
		PdhCollectQueryData(cpuQuery);

		// 갱신 데이터 얻음
		PDH_FMT_COUNTERVALUE counterVal;
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

		// 얻은 데이터 사용
		wprintf(L"CPU Usage : %f%%\n", counterVal.doubleValue);
	}

	return 0;
}

// \Processor(*)\% C1 Time