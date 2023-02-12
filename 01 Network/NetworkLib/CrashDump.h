#pragma once
#include <Windows.h>
#include <wchar.h>
#include <minidumpapiset.h>
#include <psapi.h>
#include <crtdbg.h> 
#include <errno.h>
#pragma comment (lib, "Dbghelp.lib")

struct CrashDump {
	static long _DumpCount;
	static bool wait;

	CrashDump();
	static void Crash();
	static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer);
	static void SetHandlerDump();

	// Invalid Parameter handler,
	static void mylnvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
	static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue);
	static void myPurecallHandler(void);
};