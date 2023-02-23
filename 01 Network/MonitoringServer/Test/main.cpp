#include <iostream>
#include <Windows.h>
#include <string>
using namespace std;

int main() {
	char dst[5]; //1234 0
	char src[10] = "12";

	//strncpy_s(dst, 3, src, 3);
	////dst[4] = 0;


	strncpy_s(dst, src, 4);
	cout << src << endl;
	cout << dst << endl;
}