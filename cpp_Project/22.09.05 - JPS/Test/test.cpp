#include <iostream>
#include <Windows.h>
#include <string>
#include <wchar.h>
#include <winuser.h>

using namespace std;

wchar_t buf[256] = { 0, };


int main() {
	float a = 1.5;

	swprintf_s(buf, sizeof(buf), L"%.2f", a);
	wcout << buf << endl;
	wprintf(L"%f", a);
}