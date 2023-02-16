#include <iostream>
#include <Windows.h>
#include <cstdarg>
#include <string>

using namespace std;

int main() {
	const char* str = to_string(111).c_str();
	cout << str << endl;
}
