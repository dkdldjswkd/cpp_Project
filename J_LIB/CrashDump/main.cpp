#include <iostream>
#include "CrashDump.h"

CrashDump dump;

int main() {
	int a = 1;
	CrashDump::Crash();
	a = 2;
	printf("3");
}