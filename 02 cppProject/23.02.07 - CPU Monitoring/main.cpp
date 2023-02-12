/////////////////////////////////////////////////////////////////////////////
// CCpuUsage CPUTime(); // CPUTime(hProcess)
//
// while ( 1 )
// {
//		CPUTIme.UpdateCpuTime();
//		wprintf(L"Processor:%f / Process:%f \n", CPUTime.ProcessorTotal(), CPUTime.ProcessTotal());
//		wprintf(L"ProcessorKernel:%f / ProcessKernel:%f \n", CPUTime.ProcessorKernel(), CPUTime.ProcessKernel());
//		wprintf(L"ProcessorUser:%f / ProcessUser:%f \n", CPUTime.ProcessorUser(), CPUTime.ProcessUser());
//		Sleep(1000);
// }
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <Windows.h>

// 밀리, 마이크로, 나노
// 10082843(100 나노 초 단위) == 1초
// 1008284300

int main() {
	ULARGE_INTEGER prevTime = { 0, };
	ULARGE_INTEGER curTime = { 0, };

	while (1) {
		Sleep(1000);
		GetSystemTimeAsFileTime((LPFILETIME)&curTime);
		printf("diff : %llu \n", curTime.QuadPart - prevTime.QuadPart);

		prevTime.QuadPart = curTime.QuadPart;
	}
}

// 10000 / 100mm 단위
// 