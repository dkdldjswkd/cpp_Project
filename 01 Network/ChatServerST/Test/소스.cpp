#include <iostream>
#include <strsafe.h>
using namespace std;

#define BUF_SZIE 10
char buf[BUF_SZIE];

void test() {
	va_list var_list;
	va_start(var_list, "%s %s", "12", "34");
	StringCchVPrintfA(buf, BUF_SZIE, "%s", var_list); 
	va_end(var_list);
}

int main() {
	test();
	cout << buf << endl;
}