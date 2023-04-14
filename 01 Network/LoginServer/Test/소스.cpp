#include <iostream>
#include <Windows.h>
#include <string>
#include "Parser.h"
using namespace std;
#include <cstdio>
#include <cstdarg>



int main() {
	void (*fp)() = []() {cout << 1 << endl; };
	//fp();
}