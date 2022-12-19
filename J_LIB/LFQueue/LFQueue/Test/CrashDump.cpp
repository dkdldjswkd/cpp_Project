#include "CrashDump.h"
#include "MemoryNote.h"
#include <stdio.h>

extern MemoryNote note;
long CrashDump::_DumpCount;
bool CrashDump::wait = false;

CrashDump::CrashDump() {
	_DumpCount = 0;

	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = mylnvalidParameterHandler;

	oldHandler = _set_invalid_parameter_handler(newHandler); // crt �Լ��� null ������ ���� �־�����
	_CrtSetReportMode(_CRT_WARN, 0);	// crt ���� �޽��� ǥ�� �ߴ�. �ٷ� ������ ������
	_CrtSetReportMode(_CRT_ASSERT, 0);	// crt ���� �޽��� ǥ�� �ߴ�. �ٷ� ������ ������
	_CrtSetReportMode(_CRT_ERROR, 0);	// crt ���� �޽��� ǥ�� �ߴ�. �ٷ� ������ ������

	_CrtSetReportHook(_custom_Report_hook);

	// pure virtual function called ���� �ڵ鷯 ����� ���� �Լ��� ��ȸ��Ų��.
	_set_purecall_handler(myPurecallHandler);

	SetHandlerDump();
}

void CrashDump::Crash() {
	int* p = nullptr;
	*p = 0;
}

LONG WINAPI CrashDump::MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer) {
	if (InterlockedExchange8((CHAR*)&wait, true) == true)
		Sleep(INFINITE);

	int iWorkingMemory = 0;
	SYSTEMTIME		stNowTime;
	long DumpCount = InterlockedIncrement(&_DumpCount);

	// ���� ���μ����� �޸� ��뷮�� ���´�.
	HANDLE hProcess = 0;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = GetCurrentProcess();

	if (NULL == hProcess)
		return 0;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
		iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
	}
	CloseHandle(hProcess);

	// ���� ��¥�� �ð��� �˾ƿ´�.
	WCHAR filename[MAX_PATH];

	GetLocalTime(&stNowTime);
	wsprintf(filename, L"Dump_ %d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp",
		stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount, iWorkingMemory);

	wprintf(L"\n\n\n !!! Crash Error !!! %d.%d.%d / %d:%d:%d \n",
		stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	wprintf(L"Now Save dump file... vn");

	HANDLE hDumpFile = ::CreateFile(filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDumpFile != INVALID_HANDLE_VALUE) {
		_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

		MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
		MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		MinidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&MinidumpExceptionInformation,
			NULL,
			NULL);

		CloseHandle(hDumpFile);

		wprintf(L"CrashDump Save Finish !");
	}

	// memory note write
	FILE* f;
	fopen_s(&f, "memory_log", "wb");
	if (f != 0) {
		fwrite(note.begin, note.note_size, 1, f);
		fclose(f);
	}
	else {
		printf("�޸� LOG fwrite fail \n");
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void CrashDump::SetHandlerDump() {
	SetUnhandledExceptionFilter(MyExceptionFilter);
}

void CrashDump::mylnvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved) {
	Crash();
}

int CrashDump::_custom_Report_hook(int ireposttype, char* message, int* returnvalue) {
	Crash();
	return true;
}

void CrashDump::myPurecallHandler(void) {
	Crash();
}