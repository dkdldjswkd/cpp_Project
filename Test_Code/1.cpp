#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <thread>
#pragma comment(lib, "Synchronization.lib")
using namespace std;

int key = 100;
int main() {
	int lock = 101;
	WaitOnAddress(&key, &lock, sizeof(int), INFINITE);
	printf("¹ÝÈ¯ \n");
}