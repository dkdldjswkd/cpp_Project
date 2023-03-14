#include <iostream>
#include <Windows.h>
#include "../../../00 lib_jy/ThreadCpuMonitor.h"
#include "../../../00 lib_jy/SegmentCpuMonitor.h"
#pragma comment (lib, "Winmm.lib")
using namespace std;

struct A {
	virtual void f() {
		printf("A::f()\n");
	}
};

struct B : public A {
	void f() {
		printf("B::f()\n");
		A::f();
	}
};

int main() {
	B b;
	b.f();
}