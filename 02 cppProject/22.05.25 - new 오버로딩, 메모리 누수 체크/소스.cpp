#include <iostream>
#include <string>
#include "my_new.h"

using namespace std;

class Test {
	int x;
	int y;
};

class Base {
public:
	Base() { cout << "ABC ������" << endl; }
	int a = 0xF1;
	int b = 0xF2;
	int c = 0xF3;
};

class A :public Base {
public:
	A() { cout << "A������" << endl; }
	~A() { cout << "A�Ҹ���" << endl; }
};


int main() {
	A* p1 = new A[3];


	return 0;
}