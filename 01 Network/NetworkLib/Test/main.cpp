#include <iostream>
#include <stdio.h>
#include <string.h>
#include "../../NetworkLib/FileUtils.h"
using namespace std;

char buf[] = "abcd";
bool GetValue(const char* section, const char* param, int* value) {
	char* offset = buf;
	for (;;) {
		offset = strstr(offset, section);
		if (nullptr == offset)
			return false;
		if (offset[strlen(section)] == ' ' || offset[strlen(section)] == '\t' || offset[strlen(section)] == '\n' || offset[strlen(section)] == 0)
			break;
	}

	for (;;) {
		offset = strstr(offset, "{");
		if (nullptr == offset)
			return false;
		if (offset[strlen(section)] == ' ' || offset[strlen(section)] == '\t' || offset[strlen(section)] == '\n' || offset[strlen(section)] == 0)
			break;
	}	
}

int main(){
    int value;
    GetValue("abc", "PORT", &value);
}