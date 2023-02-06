#include <iostream>
#include <Windows.h>
#include <thread>
using namespace std;

struct A {
	virtual void f() { printf("A\n"); }
};

struct B : public A {
	virtual void f() { printf("B\n"); }
};



int main() {
	A* a;
	B b;

	a = &b;
	a->f();
}