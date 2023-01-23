#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <thread>
#include <stack>
#include <mutex>
using namespace std;

//class A {
//public:
//	A() { f = f1; }
//
//private:
//	void(A::*f)();
//	void f1() { printf("f1 \n"); }
//	void f2() { printf("f2 \n"); }
//
//public:
//	void show() { f(); }
//};

WCHAR buf[100] = L"abc";

int main() {

	std::wcout << wcslen(buf) << endl;
	std::wcout << sizeof(buf) << endl;
}