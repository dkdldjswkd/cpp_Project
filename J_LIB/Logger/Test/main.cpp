#include <Windows.h>
#include <thread>
#include <iostream>
#include "Logger.h"
using namespace std;

int main() {
	LOG_INIT();

	for (;;) {
		Sleep(1000);
		LOG("battle", LOG_LEVEL_FATAL, "%d start battle  %s \n", 3, "asdf");
		LOG("battle", LOG_LEVEL_DEBUG, "%d asd \n", 3);
	}
}