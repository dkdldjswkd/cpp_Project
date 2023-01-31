#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "Logger.h"
#pragma warning(disable : 4996)
using namespace std;

struct A {
	int a;
	int b;
};

char buf[100] = "111\n";

int main() {
	LOG("HI", LOG_LEVEL_FATAL, "123 %d", 3);
	LOG("HI", LOG_LEVEL_FATAL, "123 %d", 3);
	LOG("HI", LOG_LEVEL_FATAL, "123 %d", 3);
	LOG("HI", LOG_LEVEL_FATAL, "123 %d", 3);
	printf("%s", buf);
}