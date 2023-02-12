#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{
    DWORD thread_id = GetCurrentThreadId();
    _tprintf(_T("The thread ID is %u\n"), thread_id);
    return 0;
}