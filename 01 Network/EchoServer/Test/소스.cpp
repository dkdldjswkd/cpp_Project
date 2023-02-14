#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include <string>
using namespace std;



const char* src = "";
wstring dst;

int main() {
    UTF8ToUTF16(src, dst);
	wcout << dst << endl;
	wcout << "Á¾·á" << endl;
}

