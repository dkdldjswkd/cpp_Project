#include <iostream>
#include <Windows.h>
using namespace std;

struct T {
	__declspec(align(64)) int c = 3;
};

struct Node {
	DWORD64 a =1;
	DWORD64 b = 2;
	T c;
};

int main() {
	//auto p = new Node;
	//cout << (int)(&p->c) - (int)p << endl; // T f

	WORD a = 65536;
	cout << a << endl;
}